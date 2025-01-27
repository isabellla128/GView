#include "Images.hpp"

namespace GView::GenericPlugins::Droppper::Images
{
// https://en.wikipedia.org/wiki/GIF#File_format
constexpr uint64 IMAGE_GIF_MAGIC_87a = 0x613738464947; // "GIF87a" little-endian
constexpr uint64 IMAGE_GIF_MAGIC_89a = 0x613938464947; // "GIF89a" little-endian

const std::string_view GIF::GetName() const
{
    return "GIF";
}

Category GIF::GetCategory() const
{
    return Category::Image;
}

Subcategory GIF::GetSubcategory() const
{
    return Subcategory::GIF;
}

const std::string_view GIF::GetOutputExtension() const
{
    return "gif";
}

Priority GIF::GetPriority() const
{
    return Priority::Binary;
}

bool GIF::ShouldGroupInOneFile() const
{
    return false;
}

bool GIF::Check(uint64 offset, DataCache& file, BufferView precachedBuffer, Finding& finding)
{
    // 1. Verificare semnatura "GIF87a" sau "GIF89a" (6 octeți)
    CHECK(precachedBuffer.GetLength() >= 6, false, "");
    uint64 magic = *reinterpret_cast<const uint64*>(precachedBuffer.GetData()) & 0xFFFFFFFFFFFF; // ia primii 6 octeti
    CHECK(magic == IMAGE_GIF_MAGIC_87a || magic == IMAGE_GIF_MAGIC_89a, false, "");

    finding.start = offset;
    finding.end   = offset + 6;

    // 2. Logical Screen Descriptor (7 octeți)
    auto logicalScreenDescriptor = file.CopyToBuffer(finding.end, 7, true);
    CHECK(logicalScreenDescriptor.IsValid(), false, "");

    uint16 width, height;
    uint8 gctFlag;
    memcpy(&width, logicalScreenDescriptor.GetData(), sizeof(width));
    memcpy(&height, logicalScreenDescriptor.GetData() + 2, sizeof(height));
    memcpy(&gctFlag, logicalScreenDescriptor.GetData() + 4, sizeof(gctFlag));

    CHECK(width > 0 && height > 0, false, "");
    finding.end += 7;

    // 3. Global Color Table
    auto hasGlobalColorTable = (gctFlag & 0x80) != 0;                  // verifica primul bit
    if (hasGlobalColorTable) {                                         // https://giflib.sourceforge.net/whatsinagif/bits_and_bytes.html
        auto globalColorTableSize = 3 * (1 << ((gctFlag & 0x07) + 1)); // Dimensiunea în bytes, ultimii 3 biti au dimensiunea gct (nr de intrari din tabel)
        auto globalColorTable     = file.CopyToBuffer(finding.end, globalColorTableSize, true);
        CHECK(globalColorTable.IsValid(), false, "");
        finding.end += globalColorTableSize;
    }

    // 4. blocuri de imagini, extensii, trailer
    while (true) {
        // sentinel (1 byte)
        auto sentinelBuffer = file.CopyToBuffer(finding.end, 1, true);
        CHECK(sentinelBuffer.IsValid(), false, "");
        uint8 sentinel = *sentinelBuffer.GetData();
        finding.end += 1;

        // 4.1. Trailer (0x3B, ;) - finalul fisierului
        if (sentinel == 0x3B) {
            finding.result = Result::Buffer;
            return true; // Fișier valid
        }

        // 4.2. Blocuri de extensie (0x21, !)
        else if (sentinel == 0x21) {
            auto extensionTypeBuffer = file.CopyToBuffer(finding.end, 1, true);
            CHECK(extensionTypeBuffer.IsValid(), false, "");
            finding.end += 1;

            while (true) {
                auto sizeBuffer = file.CopyToBuffer(finding.end, 1, true);
                CHECK(sizeBuffer.IsValid(), false, "");
                uint8 size = *sizeBuffer.GetData();
                finding.end += 1;
                finding.end += size;
                if (size == 0)
                    break;
            }
        }

        // 4.3. Blocuri de imagini (0x2C, ,)
        else if (sentinel == 0x2C) {
            finding.end += 8; // Avansam după primii 8 octeti din Image Descriptor, al 9-lea este local color table flag

            auto imageBuffer = file.CopyToBuffer(finding.end, 1, true);
            CHECK(imageBuffer.IsValid(), false, "");
            uint8 localColorTableFlag = *imageBuffer.GetData();
            auto hasLocalColorTable   = (localColorTableFlag & 0x80) != 0;
            finding.end += 1;

            if (hasLocalColorTable) {
                auto localColorTableSize = 3 * (1 << ((localColorTableFlag & 0x07) + 1));
                finding.end += localColorTableSize;
            }

            finding.end += 1; // LZW minimum code size, 1 byte

            while (true) {
                auto sizeBuffer = file.CopyToBuffer(finding.end, 1, true);
                CHECK(sizeBuffer.IsValid(), false, "");
                uint8 size = *sizeBuffer.GetData();
                finding.end += 1;
                finding.end += size;
                if (size == 0)
                    break;
            }
        }

        // 4.4. Bloc necunoscut
        else {
            break;
        }
    }

    finding.result = Result::Buffer;
    return true;
}

} // namespace GView::GenericPlugins::Droppper::Images
