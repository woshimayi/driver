#if 0
#include <iostream>
#include <stdexcept>
#include <vector>

class MyVector {
private:
    std::vector<int> data;

public:
    MyVector(int size) : data(size) {}

    // 常量版本
    const int& operator[](int index) const {
        if (index >= 0 && index < data.size()) {
            return data[index];
        }
        throw std::out_of_range("Index out of bounds");
    }

    // 非常量版本（用于赋值）
    int& operator[](int index) {
        if (index >= 0 && index < data.size()) {
            return data[index];
        }
        throw std::out_of_range("Index out of bounds");
    }

    int getSize() const {
        return data.size();
    }
};

int main() {
    MyVector vec(5);

    // 使用非常量 operator[] 进行赋值
    vec[0] = 10;
    vec[2] = 25;
    vec[4] = 50;

    // 使用常量 operator[] 进行读取并打印
    std::cout << "Vector elements: ";
    for (int i = 0; i < vec.getSize(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;

    // 尝试访问越界索引（会抛出异常）
    try {
        vec[10] = 100;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
#else

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <string>

class MyHash
{
private:
    std::unordered_map<std::string, int> data;

public:
    // 插入或修改键值对
    int &operator[](const std::string &key)
    {
        return data[key]; // 如果 key 不存在，会默认创建一个新的条目
    }

    // 常量版本，用于读取
    const int &operator[](const std::string &key) const
    {
        auto it = data.find(key);
        if (it != data.end())
        {
            return it->second;
        }
        throw std::out_of_range("Key not found");
    }

    bool contains(const std::string &key) const
    {
        return data.count(key) > 0;
    }

    size_t getSize() const
    {
        return data.size();
    }

    void printAll() const
    {
        std::cout << "Hash elements: ";
        for (const auto &pair : data)
        {
            std::cout << "[" << pair.first << ": " << pair.second << "] " << std::endl;
        }
        std::cout << std::endl;
    }
};

int main()
{
    MyHash myHash;

    // 使用非常量 operator[] 进行赋值（插入或修改）
    myHash["apple"] = 10;
    myHash["banana"] = 25;
    myHash["cherry"] = 50;
    myHash["apple"] = 100; // 修改 "apple" 的值

    myHash.printAll(); // 输出 Hash elements: [cherry: 50] [banana: 25] [apple: 100]

    // 使用常量 operator[] 进行读取
    std::cout << "Value of apple: " << myHash["apple"] << std::endl;
    std::cout << "Value of banana: " << myHash["banana"] << std::endl;

    // 检查键是否存在
    if (myHash.contains("grape"))
    {
        std::cout << "Value of grape: " << myHash["grape"] << std::endl;
    }
    else
    {
        std::cout << "Key 'grape' not found." << std::endl;
    }

    std::cout << "Size of hash: " << myHash.getSize() << std::endl;

    // 尝试访问不存在的键（会抛出异常）
    try
    {
        std::cout << "Value of grape: " << myHash["grape"] << std::endl;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

#endif