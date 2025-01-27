#include "png.hpp"

using namespace GView::Type::PNG;

PNGFile::PNGFile()
{
}

bool PNGFile::Update()
{
    memset(&header, 0, sizeof(header));
    
    ihdrChunk = {};
    chunks.clear();

    auto& data = this->obj->GetData();

    CHECK(data.Copy<Header>(0, header), false, "Failed to read header");

    size_t offset = sizeof(Header);
    
    while (offset < data.GetSize()) 
    {
        Chunk chunk;
        CHECK(ReadChunk(chunk, offset), false, "Failed to read chunk");

        if (chunk.type == PNG_CHUNK_TYPE_IHDR) 
        {
            CHECK(ProcessIHDRChunk(chunk, offset), false, "Failed to process IHDR chunk");
        }

        chunks.push_back(chunk);

        if (chunk.type == PNG_CHUNK_TYPE_IEND) 
        {
            break;
        }

        offset += sizeof(uint32) * 3 + chunk.length;
    }

    return true;
}

bool PNGFile::ReadChunk(Chunk& chunk, size_t offset)
{
    auto& data = this->obj->GetData();

    CHECK(data.Copy<uint32>(offset, chunk.length), false, "Failed to read chunk length");
    chunk.length = Endian::BigToNative(chunk.length);
    offset += sizeof(uint32);

    CHECK(data.Copy<uint32>(offset, chunk.type), false, "Failed to read chunk type");
    offset += sizeof(uint32);

    if (chunk.length > 0) 
    {
        chunk.data.resize(chunk.length);

        auto buffer = data.Get(offset, chunk.length, true);
        memcpy(chunk.data.data(), buffer.GetData(), chunk.length);

        offset += chunk.length;
    }

    CHECK(data.Copy<uint32>(offset, chunk.crc), false, "Failed to read chunk crc");

    return true;
}

bool PNGFile::ProcessIHDRChunk(Chunk& chunk, size_t offset)
{
    CHECK(chunk.data.size() <= sizeof(IHDRChunk), false, "IHDR chunk size is invalid");

    memcpy(&ihdrChunk, chunk.data.data(), sizeof(IHDRChunk));

    ihdrChunk.width  = Endian::BigToNative(ihdrChunk.width);
    ihdrChunk.height = Endian::BigToNative(ihdrChunk.height);

    return true;
}

bool PNGFile::LoadImageToObject(Image& img, uint32 index)
{
    Buffer buf;
    auto bf = obj->GetData().GetEntireFile();
    if (bf.IsValid() == false) {
        buf = this->obj->GetData().CopyEntireFile();
        CHECK(buf.IsValid(), false, "Fail to copy Entire file");
        bf = (BufferView) buf;
    }
    CHECK(img.Create(bf), false, "Fail to create the image object");

    return true;
}