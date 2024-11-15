#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    uint8_t signature[2] = {'B', 'M'};
    uint32_t files_size;
    uint32_t reserved_space;
    uint32_t padding;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    uint32_t header_size;
    int32_t image_width;
    int32_t image_height;
    uint16_t planes_count = 1;
    uint16_t bits_per_pixel = 24;
    uint32_t compression_type = 0;
    uint32_t bytes_in_data;
    int32_t x_pixels_per_meter = 11811;
    int32_t y_pixels_per_meter = 11811;
    int32_t used_colors = 0;
    int32_t important_colors = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
// деление, а затем умножение на 4 округляет до числа, кратного четырём
// прибавление тройки гарантирует, что округление будет вверх
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream ofs(file, ios::binary);
    if (!ofs) {
        return false;
    }

    int height = image.GetHeight();
    int width = image.GetWidth();
    int bmp_stride = GetBMPStride(width);

    BitmapFileHeader file_header;
    //Отступ данных от начала файла — 4 байта, беззнаковое целое. Он равен размеру 
    //двух частей заголовка.
    file_header.padding = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    //Зарезервированное пространство — 4 байта, заполненные нулями.
    file_header.reserved_space = 0;
    //Суммарный размер заголовка и данных — 4 байта, беззнаковое целое. Размер данных 
    //определяется как отступ, умноженный на высоту изображения.
    file_header.files_size = file_header.padding + bmp_stride * height;

    BitmapInfoHeader info_header;
    //Размер заголовка — 4 байта, беззнаковое целое. Учитывается только размер второй 
    //части заголовка.
    info_header.header_size = sizeof(BitmapInfoHeader);
    //Ширина изображения в пикселях — 4 байта, знаковое целое.
    info_header.image_width = width;
    //Высота изображения в пикселях — 4 байта, знаковое целое.
    info_header.image_height = height;
    //Количество байт в данных — 4 байта, беззнаковое целое. Произведение отступа на высоту.
    info_header.bytes_in_data = bmp_stride * height;

    ofs.write(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    ofs.write(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

    vector<char> buff(bmp_stride);
    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        ofs.write(buff.data(), bmp_stride);
    }

    ofs.close();
    return ofs.good();

}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs) {
        return {};
    }

    BitmapFileHeader file_header;
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M') {
        return {};
    }

    BitmapInfoHeader info_header;
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

    int height = info_header.image_height;
    int width = info_header.image_width;
    Image result(width, height, Color::Black());

    int bmp_stride = GetBMPStride(width);
    vector<char> buff(bmp_stride);

    for (int y = height - 1; y >= 0; --y) {
        ifs.read(buff.data(), bmp_stride);

        Color* line = result.GetLine(y);
        for (int x = 0; x < width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib