#include "png.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace GView::Utils;
using namespace GView::Type;
using namespace GView;
using namespace GView::View;

extern "C" 
{
    PLUGIN_EXPORT bool Validate(const AppCUI::Utils::BufferView& buf, const std::string_view& extension)
    {
        if (buf.GetLength() < sizeof(PNG::Header)) {
            return false;
        }

        auto header = buf.GetObject<PNG::Header>();
        if (header->magic != PNG::PNG_MAGIC_NUMBER) {
            return false;
        }

        return true;
    }

    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new PNG::PNGFile;
    }

    void CreateBufferView(Reference<GView::View::WindowInterface> win, Reference<PNG::PNGFile> png)
    {
        BufferViewer::Settings settings;

        const std::vector<ColorPair> colors = { ColorPair{ Color::Teal, Color::DarkBlue }, ColorPair{ Color::Yellow, Color::DarkBlue } };

        auto& data            = png->obj->GetData();
        const uint64 dataSize = data.GetSize();
        uint64 offset         = 0;
        uint32 colorIndex     = 0;

        settings.AddZone(0, sizeof(PNG::Header), ColorPair{ Color::Magenta, Color::DarkBlue }, "PNG Magic");

        offset += sizeof(PNG::Header);

        while (offset < dataSize) 
        {
            uint32 chunkLength, chunkType;

            data.Copy<uint32>(offset, chunkLength);
            chunkLength = Endian::BigToNative(chunkLength);

            data.Copy<uint32>(offset + 4, chunkType);

            std::string chunkLabel = "Chunk: " + std::string(reinterpret_cast<char*>(&chunkType), sizeof(uint32));
            settings.AddZone(offset, 8 + chunkLength + 4, colors[colorIndex], chunkLabel.c_str());
            offset += chunkLength + sizeof(uint32) * 3;
            colorIndex = (colorIndex + 1) % colors.size();
        }

        png->selectionZoneInterface = win->GetSelectionZoneInterfaceFromViewerCreation(settings);
    }

    void CreateImageView(Reference<GView::View::WindowInterface> win, Reference<PNG::PNGFile> png)
    {
        GView::View::ImageViewer::Settings settings;
        settings.SetLoadImageCallback(png.ToBase<View::ImageViewer::LoadImageInterface>());
        settings.AddImage(0, win->GetObject()->GetData().GetSize());
        win->CreateViewer(settings);
    }

    PLUGIN_EXPORT bool PopulateWindow(Reference<GView::View::WindowInterface> win)
    {
        auto png = win->GetObject()->GetContentType<PNG::PNGFile>();
        png->Update();

        CreateImageView(win, png);
        CreateBufferView(win, png);

        win->AddPanel(Pointer<TabPage>(new PNG::Panels::Information(png)), true);

        return true;
    }

    PLUGIN_EXPORT void UpdateSettings(IniSection sect)
    {
        sect["Pattern"]     = "magic:89 50 4E 47";
        sect["Priority"]    = 1;
        sect["Description"] = "PNG image file (*.png)";
    }
}