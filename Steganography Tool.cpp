#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#pragma pack(push, 1) // Ensure no padding
struct BMPHeader {
    uint16_t bfType;      // Magic number for BMP files
    uint32_t bfSize;      // Size of the BMP file
    uint16_t bfReserved1; // Reserved; must be zero
    uint16_t bfReserved2; // Reserved; must be zero
    uint32_t bfOffBits;   // Offset to the start of pixel data
};

struct DIBHeader {
    uint32_t biSize;          // Size of this header
    int32_t  biWidth;         // Width of the bitmap in pixels
    int32_t  biHeight;        // Height of the bitmap in pixels
    uint16_t biPlanes;        // Number of color planes
    uint16_t biBitCount;      // Number of bits per pixel
    uint32_t biCompression;    // Compression type
    uint32_t biSizeImage;     // Size of the image data
    int32_t  biXPelsPerMeter;  // Horizontal resolution
    int32_t  biYPelsPerMeter;  // Vertical resolution
    uint32_t biClrUsed;       // Number of colors in the palette
    uint32_t biClrImportant;   // Important colors
};
#pragma pack(pop)

void hideMessageInBMP(const std::string& bmpFile, const std::string& message) {
    std::ifstream input(bmpFile, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open BMP file." << std::endl;
        return;
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    // Read BMP header
    input.read(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    input.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    // Check if the file is a BMP
    if (bmpHeader.bfType != 0x4D42) { // 'BM'
        std::cerr << "Error: Not a valid BMP file." << std::endl;
        return;
    }

    // Calculate the size of the message
    size_t messageLength = message.size();
    if (messageLength > (dibHeader.biWidth * dibHeader.biHeight * dibHeader.biBitCount / 8) - 1) {
        std::cerr << "Error: Message is too long to hide in this image." << std::endl;
        return;
    }

    // Move to the pixel data
    input.seekg(bmpHeader.bfOffBits, std::ios::beg);
    std::vector<char> pixels(dibHeader.biSizeImage);
    input.read(pixels.data(), dibHeader.biSizeImage);
    input.close();

    // Hide the message in the least significant bits
    for (size_t i = 0; i < messageLength; ++i) {
        for (int j = 0; j < 8; ++j) {
            // Set the least significant bit of the pixel
            pixels[i * 8 + j] = (pixels[i * 8 + j] & 0xFE) | ((message[i] >> (7 - j)) & 0x01);
        }
    }

    // Add a null terminator to indicate the end of the message
    for (int j = 0; j < 8; ++j) {
        pixels[messageLength * 8 + j] = (pixels[messageLength * 8 + j] & 0xFE) | ((0 >> (7 - j)) & 0x01);
    }

    // Write the modified pixel data back to a new BMP file
    std::ofstream output("output.bmp", std::ios::binary);
    output.write(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    output.write(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));
    output.write(pixels.data(), dibHeader.biSizeImage);
    output.close();

    std::cout << "Message hidden successfully in output.bmp" << std::endl;
}

std::string extractMessageFromBMP(const std::string& bmpFile) {
    std::ifstream input(bmpFile, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open BMP file." << std::endl;
        return "";
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    // Read BMP header
    input.read(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    input.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    // Check if the file is a BMP
    if (bmpHeader.bfType != 0x4D42) { // 'BM'
        std::cerr << "Error: Not a valid BMP file." << std::endl;
        return "";
    }

    // Move to the pixel data
    input.seekg(bmpHeader.bfOffBits, std::ios::beg);
    std::vector<char> pixels(dibHeader.biSizeImage);
    input.read(pixels.data(), dibHeader.biSizeImage);
    input.close();

    // Extract the message
    std::string message;
    char currentChar = 0;
    for (size_t i = 0; i < dibHeader.biSizeImage * 8; ++i) {
        currentChar = (currentChar << 1) | (pixels[i] & 0x01);
        if ((i + 1) % 8 == 0) {
            if (currentChar == '\0') break; // End of message
            message += currentChar;
            currentChar = 0;
        }
    }

    return message;
}

int main() {
    int choice;
    std::string bmpFile, message;

    std::cout << "Steganography Tool\n";
    std::cout << "1. Hide Message\n";
    std::cout << "2. Extract Message\n";
    std::cout << "Choose an option: ";
    std::cin >> choice;
    std::cin.ignore(); // Ignore newline

    if (choice == 1) {
        std::cout << "Enter BMP file name: ";
        std::getline(std::cin, bmpFile);
        std::cout << "Enter message to hide: ";
        std::getline(std::cin, message);
        hideMessageInBMP(bmpFile, message);
    } else if (choice == 2) {
        std::cout << "Enter BMP file name to extract message from: ";
        std::getline(std::cin, bmpFile);
        std::string extractedMessage = extractMessageFromBMP(bmpFile);
        std::cout << "Extracted Message: " << extractedMessage << std::endl;
    } else {
        std::cout << "Invalid option." << std::endl;
    }

    return 0;
}