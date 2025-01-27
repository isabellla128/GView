#pragma once

#include "GView.hpp"

namespace GView
{
    namespace Type
    {
        namespace PNG
        {
            #pragma pack(push, 2)

            constexpr uint32 PNG_MAGIC_NUMBER    = 0x474E5089;
            constexpr uint32 PNG_CHUNK_TYPE_IHDR = 0x52444849;
            constexpr uint32 PNG_CHUNK_TYPE_IEND = 0x49454E44;

            struct Header {
                uint32 magic;
                uint16 carriageReturn_LineFeed;
                uint8 fileTerminator;
                uint8 lineFeed;
            };

            struct Chunk {
                uint32 length;
                uint32 type;

                std::vector<uint8> data;

                uint32 crc;
            };

            #pragma pack(push, 1)
            
            struct IHDRChunk {
                uint8 interlaceMethod;
                uint8 filterMethod;
                uint8 compressionMethod;
                uint8 colorType;
                uint8 bitDepth;
                uint32 height;
                uint32 width;
            };

            #pragma pack(pop)

            class PNGFile : public TypeInterface, public View::ImageViewer::LoadImageInterface
            {
              public:
                Header header{};
                IHDRChunk ihdrChunk{};
                std::vector<Chunk> chunks;

                Reference<GView::Utils::SelectionZoneInterface> selectionZoneInterface;

              public:
                PNGFile();
                virtual ~PNGFile()
                {
                }

                std::string_view GetTypeName() override
                {
                    return "PNG";
                }

                void RunCommand(std::string_view) override
                {
                }

                bool UpdateKeys(KeyboardControlsInterface* interface) override
                {
                    return true;
                }

                bool Update();

                bool ReadChunk(Chunk& chunk, size_t offset);

                bool ProcessIHDRChunk(Chunk& chunk, size_t offset);

                bool LoadImageToObject(Image& img, uint32 index) override;

                uint32 GetSelectionZonesCount() override
                {
                    CHECK(selectionZoneInterface.IsValid(), 0, "");
                    return selectionZoneInterface->GetSelectionZonesCount();
                }

                TypeInterface::SelectionZone GetSelectionZone(uint32 index) override
                {
                    static auto d = TypeInterface::SelectionZone{ 0, 0 };
                    CHECK(selectionZoneInterface.IsValid(), d, "");
                    CHECK(index < selectionZoneInterface->GetSelectionZonesCount(), d, "");

                    return selectionZoneInterface->GetSelectionZone(index);
                }

                Reference<GView::Utils::SelectionZoneInterface> GetSelectionZoneInterface()
                {
                    return selectionZoneInterface;
                }
            };

            namespace Panels
            {
                class Information : public AppCUI::Controls::TabPage
                {
                    Reference<GView::Type::PNG::PNGFile> png;
                    Reference<AppCUI::Controls::ListView> general;
                    Reference<AppCUI::Controls::ListView> issues;

                    void UpdateGeneralInformation();
                    void UpdateIssues();
                    void RecomputePanelsPositions();

                  public:
                    Information(Reference<GView::Type::PNG::PNGFile> png);

                    void Update();
                    virtual void OnAfterResize(int newWidth, int newHeight) override
                    {
                        RecomputePanelsPositions();
                    }
                };
            }; // namespace Panels
        } // namespace PNG
    } // namespace Type
} // namespace GView