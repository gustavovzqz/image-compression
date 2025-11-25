#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <queue>
#include <unordered_map>
#include <vector>
namespace fs = std::filesystem;

fs::path PACKED_TREE_PATH = "packed_tree.bin";
fs::path RLE_TREE_PATH = "rle_tree.bin";
fs::path PACKED_DATA_PATH = "packed_data.bin";
fs::path RLE_DATA_PATH = "rle_data.bin";
fs::path RLE_BIN_PATH = "rle.bin";

void buildFreqList(std::unordered_map<uint16_t, size_t> &packed_freq,
                   std::unordered_map<uint8_t, size_t> &rle_freq) {
  std::ifstream file(RLE_BIN_PATH, std::ios::binary);
  if (!file) {
    throw std::invalid_argument("rle not created");
  }

  int width, height;
  file.read(reinterpret_cast<char *>(&width), sizeof(int));
  file.read(reinterpret_cast<char *>(&height), sizeof(int));

  cv::Mat imgHSV(height, width, CV_8UC3);

  int totalPixels = width * height;
  int idx = 0;

  while (idx < totalPixels && file) {
    uint16_t packed;
    uint8_t runLength;

    file.read(reinterpret_cast<char *>(&packed), sizeof(uint16_t));
    file.read(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));
    packed_freq[packed]++;
    rle_freq[runLength]++;
  }

  file.close();
}

template <typename T> struct Node {
  T data;
  size_t frequency;
  Node *left;
  Node *right;

  Node(T d, size_t freq)
      : data(d), frequency(freq), left(nullptr), right(nullptr) {}
  Node(size_t freq)
      : data(T(0)), frequency(freq), left(nullptr), right(nullptr) {}
};

template <typename T> void deleteTree(Node<T> *root) {
  if (!root)
    return;
  deleteTree(root->left);
  deleteTree(root->right);
  delete root;
}

template <typename T> class Compare {
public:
  bool operator()(Node<T> *a, Node<T> *b) {
    return a->frequency > b->frequency; // min-heap
  }
};

template <typename T>
void preOrder(Node<T> *root, std::unordered_map<T, std::string> &table,
              std::string curr) {
  if (!root)
    return;

  if (!root->left && !root->right) {
    table[root->data] = curr;
    return;
  }

  preOrder(root->left, table, curr + "0");
  preOrder(root->right, table, curr + "1");
}

template <typename T>
std::unordered_map<T, std::string>
huffmanCodes(Node<T> *&root, std::unordered_map<T, size_t> &freqMap) {
  std::priority_queue<Node<T> *, std::vector<Node<T> *>, Compare<T>> pq;

  for (const auto &[key, value] : freqMap)
    pq.push(new Node<T>(key, value));

  while (pq.size() > 1) {
    Node<T> *l = pq.top();
    pq.pop();
    Node<T> *r = pq.top();
    pq.pop();
    Node<T> *parent = new Node<T>(l->frequency + r->frequency);
    parent->left = l;
    parent->right = r;
    pq.push(parent);
  }

  root = pq.top();
  std::unordered_map<T, std::string> table;
  preOrder(root, table, "");
  return table;
}

template <typename T>
void writeTree(Node<T> *root, const std::string &filename) {
  std::ofstream out(filename, std::ios::binary);
  if (!out) {
    throw std::runtime_error(
        "Não foi possível abrir o arquivo para escrever a árvore");
  }

  std::function<void(Node<T> *)> writeNode = [&](Node<T> *node) {
    if (!node)
      return;

    if (!node->left && !node->right) {
      out.put(1);
      out.write(reinterpret_cast<char *>(&node->data), sizeof(T));
    } else {
      out.put(0);
      writeNode(node->left);
      writeNode(node->right);
    }
  };

  writeNode(root);
  out.close();
}

template <typename T>
void writeHuffmanData(const std::vector<T> &data,
                      const std::unordered_map<T, std::string> &table,
                      const std::string &filename) {
  std::ofstream out(filename, std::ios::binary);
  if (!out)
    throw std::runtime_error("Não foi possível abrir arquivo");

  uint8_t buffer = 0;
  int bitsFilled = 0;

  for (T symbol : data) {
    const std::string &code = table.at(symbol);
    for (char c : code) {
      buffer <<= 1;
      if (c == '1')
        buffer |= 1;
      bitsFilled++;

      if (bitsFilled == 8) {
        out.put(buffer);
        bitsFilled = 0;
        buffer = 0;
      }
    }
  }

  if (bitsFilled > 0) {
    buffer <<= (8 - bitsFilled);
    out.put(buffer);
  }

  out.close();
}

void unifyFiles(int width, int height, std::string path) {
  std::vector<std::string> files = {PACKED_TREE_PATH, RLE_TREE_PATH,
                                    PACKED_DATA_PATH, RLE_DATA_PATH};

  std::ofstream out(path, std::ios::binary);
  if (!out)
    throw std::runtime_error("Erro na criação do arquivo comprimido");

  out.write(reinterpret_cast<char *>(&width), sizeof(int));
  out.write(reinterpret_cast<char *>(&height), sizeof(int));

  for (const auto &file : files) {
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    if (!in)
      throw std::runtime_error("Não foi possível abrir " + file);

    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    in.read(buffer.data(), size);

    uint32_t s = static_cast<uint32_t>(size);
    out.write(reinterpret_cast<char *>(&s), sizeof(uint32_t));
    out.write(buffer.data(), size);

    in.close();
  }

  out.close();

  fs::remove(PACKED_DATA_PATH);
  fs::remove(RLE_DATA_PATH);
  fs::remove(PACKED_TREE_PATH);
  fs::remove(RLE_TREE_PATH);
}

void runHuffman(std::string path) {
  std::unordered_map<uint16_t, size_t> packed_freq;
  std::unordered_map<uint8_t, size_t> rle_freq;

  buildFreqList(packed_freq, rle_freq);

  Node<uint16_t> *root_packed = nullptr;
  Node<uint8_t> *root_rle = nullptr;

  auto packedTable = huffmanCodes<uint16_t>(root_packed, packed_freq);
  auto rleTable = huffmanCodes<uint8_t>(root_rle, rle_freq);

  writeTree<uint16_t>(root_packed, PACKED_TREE_PATH);
  writeTree<uint8_t>(root_rle, RLE_TREE_PATH);

  std::vector<uint16_t> packedData;
  std::vector<uint8_t> rleData;

  std::ifstream file(RLE_BIN_PATH, std::ios::binary);
  if (!file)
    throw std::runtime_error("Não foi possível abrir rle.bin");

  int width, height;
  file.read(reinterpret_cast<char *>(&width), sizeof(int));
  file.read(reinterpret_cast<char *>(&height), sizeof(int));
  int totalPixels = width * height;

  for (int i = 0; i < totalPixels && file; i++) {
    uint16_t packed;
    uint8_t runLength;
    file.read(reinterpret_cast<char *>(&packed), sizeof(uint16_t));
    file.read(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));
    packedData.push_back(packed);
    rleData.push_back(runLength);
  }
  file.close();

  writeHuffmanData(packedData, packedTable, PACKED_DATA_PATH);
  writeHuffmanData(rleData, rleTable, RLE_DATA_PATH);
  unifyFiles(width, height, path);
  fs::remove(RLE_BIN_PATH);
  deleteTree(root_packed);
  deleteTree(root_rle);
}

template <typename T> Node<T> *readTree(std::ifstream &in) {
  if (!in)
    return nullptr;

  char flag;
  in.get(flag);
  if (!in)
    return nullptr;

  if (flag == 1) {
    T value;
    in.read(reinterpret_cast<char *>(&value), sizeof(T));
    return new Node<T>(value, 0);
  } else {
    Node<T> *node = new Node<T>(0, 0);
    node->left = readTree<T>(in);
    node->right = readTree<T>(in);
    return node;
  }
}

std::vector<bool> readBits(const std::string &filename) {
  std::ifstream in(filename, std::ios::binary);
  if (!in)
    throw std::runtime_error("Não foi possível abrir arquivo " + filename);

  std::vector<bool> bits;
  char byte;
  while (in.get(byte)) {
    for (int i = 7; i >= 0; --i)
      bits.push_back((byte >> i) & 1);
  }
  return bits;
}

template <typename T>
std::vector<T> decodeHuffman(Node<T> *root, const std::vector<bool> &bits,
                             size_t expectedSymbols) {
  std::vector<T> output;
  Node<T> *node = root;

  for (bool bit : bits) {
    node = bit ? node->right : node->left;

    if (!node->left && !node->right) {
      output.push_back(node->data);
      node = root;
      if (output.size() == expectedSymbols)
        break;
    }
  }

  return output;
}

void splitUnified(int &width, int &height, std::string unifiedFile) {
  std::ifstream in(unifiedFile, std::ios::binary);
  if (!in)
    throw std::runtime_error("Não foi possível abrir ");

  in.read(reinterpret_cast<char *>(&width), sizeof(int));
  in.read(reinterpret_cast<char *>(&height), sizeof(int));

  std::vector<std::string> files = {PACKED_TREE_PATH, RLE_TREE_PATH,
                                    PACKED_DATA_PATH, RLE_DATA_PATH};

  for (const auto &file : files) {
    uint32_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint32_t));

    std::vector<char> buffer(size);
    in.read(buffer.data(), size);

    std::ofstream out(file, std::ios::binary);
    out.write(buffer.data(), size);
    out.close();
  }

  in.close();
}

void reconstructOriginal(std::string unifiedFile, std::string output) {
  int width;
  int height;

  splitUnified(width, height, unifiedFile);

  int totalSymbols = width * height;

  std::ifstream inPackedTree(PACKED_TREE_PATH, std::ios::binary);
  std::ifstream inRleTree(RLE_TREE_PATH, std::ios::binary);
  if (!inPackedTree || !inRleTree)
    throw std::runtime_error("Não foi possível abrir arquivo de árvore");

  Node<uint16_t> *rootPacked = readTree<uint16_t>(inPackedTree);
  Node<uint8_t> *rootRle = readTree<uint8_t>(inRleTree);

  inPackedTree.close();
  inRleTree.close();

  auto packedBits = readBits(PACKED_DATA_PATH);
  auto rleBits = readBits(RLE_DATA_PATH);

  auto packedData = decodeHuffman(rootPacked, packedBits, totalSymbols);
  auto rleData = decodeHuffman(rootRle, rleBits, totalSymbols);

  std::ofstream out(output, std::ios::binary);
  if (!out)
    throw std::runtime_error("Não foi possível criar arquivo de saída");

  out.write(reinterpret_cast<char *>(&width), sizeof(int));
  out.write(reinterpret_cast<char *>(&height), sizeof(int));

  for (size_t i = 0; i < packedData.size(); i++) {
    out.write(reinterpret_cast<char *>(&packedData[i]), sizeof(uint16_t));
    out.write(reinterpret_cast<char *>(&rleData[i]), sizeof(uint8_t));
  }

  out.close();

  deleteTree(rootPacked);
  deleteTree(rootRle);
  fs::remove(PACKED_DATA_PATH);
  fs::remove(RLE_DATA_PATH);
  fs::remove(PACKED_TREE_PATH);
  fs::remove(RLE_TREE_PATH);
}
