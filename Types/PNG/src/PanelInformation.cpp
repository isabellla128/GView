#include "png.hpp"

using namespace GView::Type::PNG;
using namespace AppCUI::Controls;

const char* GetBitDepthDescription(uint8 bitDepth);
const char* GetColorTypeDescription(uint8 colorType);
const char* GetCompressionMethodDescription(uint8 compressionMethod);
const char* GetFilterMethodDescription(uint8 filterMethod);
const char* GetInterlaceMethodDescription(uint8 interlaceMethod);

Panels::Information::Information(Reference<GView::Type::PNG::PNGFile> _png) : TabPage("&Information")
{
    png     = _png;
    general = Factory::ListView::Create(this, "x:0,y:0,w:100%,h:10", { "n:Field,w:12", "n:Value,w:100" }, ListViewFlags::None);

    issues = Factory::ListView::Create(this, "x:0,y:21,w:100%,h:10", { "n:Info,w:200" }, ListViewFlags::HideColumns);

    this->Update();
}

void Panels::Information::UpdateGeneralInformation()
{
    LocalString<256> tempStr;
    NumericFormatter n;

    general->DeleteAllItems();
    general->AddItem("File");

    general->AddItem({ "Size", tempStr.Format("%s bytes", n.ToString(png->obj->GetData().GetSize(), { NumericFormatFlags::None, 10, 3, ',' }).data()) });
    general->AddItem({ "IHDRChunk Size", tempStr.Format("%d bytes", sizeof(IHDRChunk)) });

    general->AddItem({ "Width", tempStr.Format("%d", png->ihdrChunk.width) });
    general->AddItem({ "Height", tempStr.Format("%d", png->ihdrChunk.height) });

    const char* bitDepthDescription = GetBitDepthDescription(png->ihdrChunk.bitDepth);
    general->AddItem({ "Bit Depth", tempStr.Format("%s (%d)", bitDepthDescription, png->ihdrChunk.bitDepth) });

    const char* colorTypeDescription = GetColorTypeDescription(png->ihdrChunk.colorType);
    general->AddItem({ "Color Type", tempStr.Format("%s (%d)", colorTypeDescription, png->ihdrChunk.colorType) });
    
    const char* compressionMethod = GetCompressionMethodDescription(png->ihdrChunk.compressionMethod);
    general->AddItem({ "Compression Method", tempStr.Format("%s (%d)", compressionMethod, png->ihdrChunk.compressionMethod) });
    
    const char* filterMethod = GetFilterMethodDescription(png->ihdrChunk.filterMethod);
    general->AddItem({ "Filter Method", tempStr.Format("%s (%d)", filterMethod, png->ihdrChunk.filterMethod) });

    const char* interlaceMethodDescription = GetInterlaceMethodDescription(png->ihdrChunk.interlaceMethod);
    general->AddItem({ "Interlace Method", tempStr.Format("%s (%d)", interlaceMethodDescription, png->ihdrChunk.interlaceMethod) });
}

void Panels::Information::UpdateIssues()
{
}

void Panels::Information::RecomputePanelsPositions()
{
    int py = 0;
    int w  = this->GetWidth();
    int h  = this->GetHeight();

    if ((!general.IsValid()) || (!issues.IsValid())) {
        return;
    }

    issues->SetVisible(false);
    this->general->Resize(w, h);
}

void Panels::Information::Update()
{
    UpdateGeneralInformation();
    UpdateIssues();
    RecomputePanelsPositions();
}
const char* GetBitDepthDescription(uint8 bitDepth)
{
    switch (bitDepth) {
    case 1:
        return "1 bit per channel (2 shades: black and white)";
    case 2:
        return "2 bits per channel (4 shades)";
    case 4:
        return "4 bits per channel (16 shades)";
    case 8:
        return "8 bits per channel (256 shades or colors)";
    case 16:
        return "16 bits per channel (65,536 shades or colors, high precision)";
    default:
        return "Unknown";
    }
}

const char* GetColorTypeDescription(uint8 colorType)
{
    switch (colorType) {
    case 0:
        return "Grayscale";
    case 2:
        return "Truecolor";
    case 3:
        return "Indexed-color";
    case 4:
        return "Grayscale with alpha";
    case 6:
        return "Truecolor with alpha";
    default:
        return "Unknown";
    }
}

const char* GetCompressionMethodDescription(uint8 compressionMethod)
{
    return compressionMethod == 0 ? "Deflate" : "Unknown";
}

const char* GetFilterMethodDescription(uint8 filterMethod)
{
    return filterMethod == 0 ? "Adaptive" : "Unknown";
}

const char* GetInterlaceMethodDescription(uint8 interlaceMethod)
{
    switch (interlaceMethod) {
    case 0:
        return "No interlace";
    case 1:
        return "Adam7 interlace";
    default:
        return "Unknown";
    }
}