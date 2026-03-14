#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <random>
#include <limits>
#include <sstream>
using namespace std;

int rounds;
vector<string> subKeys;
string merged = "";

// S-Box الثابت (4-bit)
const int S_BOX[16] = {
    0xC, 0x5, 0x6, 0xB,
    0x9, 0x0, 0xA, 0xD,
    0x3, 0xE, 0xF, 0x8,
    0x4, 0x7, 0x1, 0x2
};

const int INV_S_BOX[16] = {
    0x5, 0xE, 0xF, 0x8,
    0xC, 0x1, 0x2, 0xD,
    0xB, 0x4, 0x6, 0x3,
    0x0, 0x7, 0x9, 0xA
};

// ==================== إعلانات الدوال ====================
string toHex(const string& data);
string processSBox(const string& input, bool isEncrypt);
string keyGener(int keysize);
vector<string> generateSubKeys(string& keyMaster, int rounds);
int getRounds(int keySizeBytes);
string applyXOR(const string& data, const string& key);

// دوال S-Box الديناميكي
vector<unsigned char> generateSBox(const string& key);
vector<unsigned char> generateInvSBox(const vector<unsigned char>& sbox);
string applyKeySBox(const string& input, const vector<unsigned char>& sbox);
string finalChaosSBox(const string& input, const vector<unsigned char>& sbox, 
                      const vector<unsigned char>& inv_sbox, bool encrypt);

// دوال P-Box
string applyPBox(const string& input);

// دوال Final Chaos المعقدة
string finalChaosComplex(const string& input, bool encrypt);

// دوال تقسيم الكتل
vector<vector<vector<unsigned char>>> splitBlocks(string data, int x);
string applyRounds(string text);
string disRounds(string text);

// ==================== الدالة الرئيسية ====================
int main(int argc, char* argv[]) {
    // التحقق من المدخلات
    if(argc < 3) {
        cerr << "{\"error\":\"Invalid arguments\"}" << endl;
        return 1;
    }
    
    int keySizeChoice = atoi(argv[1]);
    string text = argv[2];
    
    int keysize;
    if(keySizeChoice == 1) keysize = 16;
    else if(keySizeChoice == 2) keysize = 32;
    else if(keySizeChoice == 3) keysize = 64;
    else {
        cerr << "{\"error\":\"Invalid key size\"}" << endl;
        return 1;
    }
    
    try {
        // ========== بداية عملية التشفير ==========
        string masterKey = keyGener(keysize);
        vector<unsigned char> sbox = generateSBox(masterKey);
        vector<unsigned char> inv_sbox = generateInvSBox(sbox);
        
        rounds = getRounds(keysize);
        subKeys = generateSubKeys(masterKey, rounds);

        // عملية التشفير
        string encrypted = text;
        vector<string> encryptSteps;
        vector<string> encryptValues;  // لحفظ القيم المشفرة لكل خطوة

        // خطوة 1: Static S-Box Encryption
        encrypted = processSBox(encrypted, true);
        encryptSteps.push_back("Static S-Box Encryption");
        encryptValues.push_back(toHex(encrypted));
       
        // خطوة 2: P-Box Encryption
        encrypted = applyPBox(encrypted);
        encryptSteps.push_back("P-Box Encryption");
        encryptValues.push_back(toHex(encrypted));
        
        // خطوات 3 إلى N: XOR مع المفاتيح الفرعية
        for(int round = 0; round < rounds; round++) {
            encrypted = applyXOR(encrypted, subKeys[round]);
            encryptSteps.push_back(string("XOR with SubKey ") + to_string(round + 1));
            encryptValues.push_back(toHex(encrypted));
        }
        
        // خطوة N+1: Dynamic S-Box Encryption
        encrypted = finalChaosSBox(encrypted, sbox, inv_sbox, true);
        encryptSteps.push_back("Dynamic S-Box Encryption");
        encryptValues.push_back(toHex(encrypted));

        // خطوة N+2: Final Chaos Complex Encryption
        encrypted = finalChaosComplex(encrypted, true);
        encryptSteps.push_back("Final Chaos Complex Encryption");
        encryptValues.push_back(toHex(encrypted));
        
        // خطوة N+3: Block System Encryption
        encrypted = applyRounds(encrypted);
        encryptSteps.push_back("Block System Encryption");
        encryptValues.push_back(toHex(encrypted));

        // عملية فك التشفير
        string decrypted = encrypted;
        vector<string> decryptSteps;
        vector<string> decryptValues;  // لحفظ القيم المفكوكة لكل خطوة

        // خطوة 1: Block System Decryption
        decrypted = disRounds(decrypted);
        decryptSteps.push_back("Block System Decryption");
        decryptValues.push_back(toHex(decrypted));
        
        // خطوة 2: Final Chaos Complex Decryption
        decrypted = finalChaosComplex(decrypted, false);
        decryptSteps.push_back("Final Chaos Complex Decryption");
        decryptValues.push_back(toHex(decrypted));

        // خطوة 3: Dynamic S-Box Decryption
        decrypted = finalChaosSBox(decrypted, sbox, inv_sbox, false);
        decryptSteps.push_back("Dynamic S-Box Decryption");
        decryptValues.push_back(toHex(decrypted));
        
        // خطوات 4 إلى N: XOR مع المفاتيح الفرعية (بترتيب عكسي)
        for(int round = rounds-1; round >= 0; round--) {
            decrypted = applyXOR(decrypted, subKeys[round]);
            decryptSteps.push_back(string("XOR with SubKey ") + to_string(round + 1));
            decryptValues.push_back(toHex(decrypted));
        }
        
        // خطوة N+1: P-Box Decryption
        decrypted = applyPBox(decrypted);
        decryptSteps.push_back("P-Box Decryption");
        decryptValues.push_back(toHex(decrypted));
        
        // خطوة N+2: Static S-Box Decryption
        decrypted = processSBox(decrypted, false);
        decryptSteps.push_back("Static S-Box Decryption");
        decryptValues.push_back(toHex(decrypted));
        
        // بناء JSON النتيجة
        stringstream json;
        json << "{";
        json << "\"success\":true,";
        json << "\"originalText\":\"" << text << "\",";
        json << "\"masterKey\":\"" << toHex(masterKey) << "\",";
        json << "\"subKeys\":[";
        for(size_t i = 0; i < subKeys.size(); i++) {
            json << "\"" << toHex(subKeys[i]) << "\"";
            if(i < subKeys.size() - 1) json << ",";
        }
        json << "],";
        
        // خطوات التشفير مع القيم
        json << "\"stepsEncrypt\":[";
        for(size_t i = 0; i < encryptSteps.size(); i++) {
            json << "{";
            json << "\"stepNumber\":" << (i + 1) << ",";
            json << "\"stepName\":\"" << encryptSteps[i] << "\",";
            json << "\"value\":\"" << encryptValues[i] << "\"";
            json << "}";
            if(i < encryptSteps.size() - 1) json << ",";
        }
        json << "],";
        
        // خطوات فك التشفير مع القيم
        json << "\"stepsDecrypt\":[";
        for(size_t i = 0; i < decryptSteps.size(); i++) {
            json << "{";
            json << "\"stepNumber\":" << (i + 1) << ",";
            json << "\"stepName\":\"" << decryptSteps[i] << "\",";
            json << "\"value\":\"" << decryptValues[i] << "\"";
            json << "}";
            if(i < decryptSteps.size() - 1) json << ",";
        }
        json << "],";
        
        json << "\"totalStepsEncrypt\":" << encryptSteps.size() << ",";
        json << "\"totalStepsDecrypt\":" << decryptSteps.size() << ",";
        json << "\"finalEncrypted\":\"" << toHex(encrypted) << "\",";
        json << "\"finalDecrypted\":\"" << decrypted << "\",";
        json << "\"match\":" << (decrypted == text ? "true" : "false");
        json << "}";
        
        cout << json.str() << endl;
        return 0;
        
    } catch(exception& e) {
        cerr << "{\"error\":\"" << e.what() << "\"}" << endl;
        return 1;
    }
}

// ==================== تعريف الدوال ====================

string toHex(const string& data) {
    stringstream ss;
    for(unsigned char c : data) {
        ss << hex << setw(2) << setfill('0') << (int)c;
    }
    return ss.str();
}

string processSBox(const string& input, bool isEncrypt) {
    string output;
    for(unsigned char c : input) {
        unsigned char high = (c >> 4) & 0x0F;
        unsigned char low = c & 0x0F;
        
        if(isEncrypt) {
            high = S_BOX[high];
            low = S_BOX[low];
        } else {
            high = INV_S_BOX[high];
            low = INV_S_BOX[low];
        }
        
        output += (high << 4) | low;
    }
    return output;
}

string keyGener(int keysize) {
    string Masterkey = "";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 255);
    
    for(int i = 0; i < keysize; i++) {
        Masterkey += char(dist(gen));
    }
    
    return Masterkey;
}

int getRounds(int keySizeBytes) {
    if(keySizeBytes == 16) return 11;
    else if(keySizeBytes == 32) return 13;
    else return 15;
}

string applyXOR(const string& data, const string& key) {
    string result = data;
    for(size_t i = 0; i < data.size(); i++) {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

vector<string> generateSubKeys(string& keyMaster, int rounds) {
    vector<string> subKeys;
    
    if(rounds == 11) {
        for(int padding = (int)keyMaster.size(), val = 0x01; padding < 32; padding++, val++) {
            keyMaster += char(val);
        }
    }

    int rows = 4;
    int cols = (int)keyMaster.size() / rows;
    if(cols * rows < (int)keyMaster.size()) cols++;

    vector<vector<unsigned char>> matrix(rows, vector<unsigned char>(cols, 0));
    int index = 0;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            if(index < (int)keyMaster.size()) {
                matrix[i][j] = keyMaster[index++];
            }
        }
    }

    vector<unsigned char> fixedKey(32);
    for(int i = 0; i < 32; i++) {
        fixedKey[i] = matrix[i % rows][0];
    }

    string previousSubKey = "";
    for(int round = 1; round <= rounds; round++) {
        string subKey = "";
        for(int row = 0; row < rows; row++) {
            int shift = (row + round) % cols;
            for(int j = 0; j < cols && (int)subKey.size() < 32; j++) {
                unsigned char val = matrix[row][(j + shift) % cols];
                if(round == 1) {
                    val ^= fixedKey[(row * cols + j) % 32];
                } else {
                    val ^= previousSubKey[(row * cols + j) % 32];
                }
                subKey += val;
            }
        }
        subKeys.push_back(subKey);
        previousSubKey = subKey;
    }

    return subKeys;
}

// ========== دوال S-Box الديناميكي ==========
vector<unsigned char> generateSBox(const string& key) {
    vector<unsigned char> sbox(256);
    
    for(int i = 0; i < 256; i++) {
        sbox[i] = i;
    }
    
    int j = 0;
    for(int i = 0; i < 256; i++) {
        j = (j + sbox[i] + (unsigned char)key[i % key.size()]) % 256;
        swap(sbox[i], sbox[j]);
    }
    
    return sbox;
}

vector<unsigned char> generateInvSBox(const vector<unsigned char>& sbox) {
    vector<unsigned char> inv_sbox(256);
    
    for(int i = 0; i < 256; i++) {
        inv_sbox[sbox[i]] = i;
    }
    
    return inv_sbox;
}

string applyKeySBox(const string& input, const vector<unsigned char>& sbox) {
    string output;
    for(unsigned char c : input) {
        output += sbox[c];
    }
    return output;
}

string finalChaosSBox(const string& input, const vector<unsigned char>& sbox, 
                      const vector<unsigned char>& inv_sbox, bool encrypt) {
    if(encrypt) {
        return applyKeySBox(input, sbox);
    } else {
        return applyKeySBox(input, inv_sbox);
    }
}

// ========== دوال P-Box ==========
string applyPBox(const string& input) {
    string output = input;
    for(int i = 0; i < (int)input.size(); i++) {
        output[i] = input[input.size() - 1 - i];
    }
    return output;
}

// ========== دوال Final Chaos المعقدة ==========
string finalChaosComplex(const string& input, bool encrypt) {
    string output = input;
    int len = (int)output.size();
    
    if(encrypt) {
        for(int i = 0; i < len-1; i += 2) {
            swap(output[i], output[i+1]);
        }
        
        for(int i = 0; i < len/2; i++) {
            swap(output[i], output[len-1-i]);
        }
        
        if(len > 0) {
            string temp = output;
            for(int i = 0; i < len; i++) {
                output[(i + 3) % len] = temp[i];
            }
        }
        
        for(int i = 0; i < len; i++) {
            if(i % 2 == 0) {
                output[i] = output[i] + 5;
            } else {
                output[i] = output[i] - 3;
            }
        }
        
        if(len > 0) {
            string temp = output;
            for(int i = 0; i < len; i++) {
                output[i] = temp[(i + 2) % len];
            }
        }
    } else {
        if(len > 0) {
            string temp = output;
            for(int i = 0; i < len; i++) {
                output[(i + 2) % len] = temp[i];
            }
        }
        
        for(int i = 0; i < len; i++) {
            if(i % 2 == 0) {
                output[i] = output[i] - 5;
            } else {
                output[i] = output[i] + 3;
            }
        }
        
        if(len > 0) {
            string temp = output;
            for(int i = 0; i < len; i++) {
                output[i] = temp[(i + 3) % len];
            }
        }
        
        for(int i = 0; i < len/2; i++) {
            swap(output[i], output[len-1-i]);
        }
        
        for(int i = 0; i < len-1; i += 2) {
            swap(output[i], output[i+1]);
        }
    }
    
    return output;
}

// ========== دوال تقسيم الكتل ==========
vector<vector<vector<unsigned char>>> splitBlocks(string data, int x) {
    vector<vector<vector<unsigned char>>> blocks(4, vector<vector<unsigned char>>(4, vector<unsigned char>(4)));

    int index = 0, pad = 1;

    for(int b = 0; b < 4; b++) {
        vector<unsigned char> temp;

        for(int i = 0; i < 8 && index < (int)data.size(); i++)
            temp.push_back(data[index++]);

        while((int)temp.size() < 16)
            temp.push_back(pad++);

        int k = 0;
        for(int r = 0; r < 4; r++) {
            for(int c = 0; c < 4; c++) {
                blocks[b][r][c] = temp[k++];
                if(x == 2) merged += blocks[b][r][c];
            }
        }
    }
    
    if(x == 2 && (int)merged.size() > (int)data.size())
        merged = merged.substr(0, data.size());
        
    return blocks;
}

string applyRounds(string text) {
    for(int round = 0; round < rounds; round++) {
        splitBlocks(text, 1);
        text = applyXOR(text, subKeys[round]);
    }
    return text;
}

string disRounds(string text) {
    splitBlocks(text, 2);
    for(int round = rounds-1; round >= 0; round--) {
        text = applyXOR(text, subKeys[round]);
    }
    return text;
}