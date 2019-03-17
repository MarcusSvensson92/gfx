#include "GfxInternal.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static Blob CreateModelBlob(const void* data, size_t size, const char* material_dir)
{
    Blob blob;
    blob.m_Data = NULL;
    blob.m_Size = 0;
    
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::stringstream ss;
    ss.write(static_cast<const char*>(data), size);
    tinyobj::MaterialFileReader mat_file_reader(material_dir);
    if (!tinyobj::LoadObj(shapes, materials, err, ss, mat_file_reader))
    {
        Print(err.c_str());
        return blob;
    }
    
    const uint32_t shape_count = static_cast<uint32_t>(shapes.size());
    const uint32_t material_count = static_cast<uint32_t>(materials.size());
    
    uint32_t total_vertex_count = 0;
    uint32_t total_triangle_count = 0;
    for (uint32_t i = 0; i < shape_count; ++i)
    {
        total_vertex_count += static_cast<uint32_t>(shapes[i].mesh.positions.size() / 3);
        total_triangle_count += static_cast<uint32_t>(shapes[i].mesh.indices.size() / 3);
    }
    
    glm::vec3* positions = static_cast<glm::vec3*>(Alloc(sizeof(glm::vec3) * total_vertex_count));
    glm::vec2* texcoords = static_cast<glm::vec2*>(Alloc(sizeof(glm::vec2) * total_vertex_count));
    glm::vec3* normals = static_cast<glm::vec3*>(Alloc(sizeof(glm::vec3) * total_vertex_count));
    uint32_t* indices = static_cast<uint32_t*>(Alloc(sizeof(uint32_t) * 3 * total_triangle_count));
    
    glm::vec3 bounding_box_min = glm::vec3(FLT_MAX);
    glm::vec3 bounding_box_max = glm::vec3(-FLT_MAX);
    
    for (uint32_t i = 0, vertex_offset = 0, triangle_offset = 0; i < shape_count; ++i)
    {
        const tinyobj::mesh_t& mesh = shapes[i].mesh;
    
        const uint32_t vertex_count = static_cast<uint32_t>(mesh.positions.size() / 3);
        const uint32_t triangle_count = static_cast<uint32_t>(mesh.indices.size() / 3);
    
        for (uint32_t j = 0; j < vertex_count; ++j)
        {
            positions[vertex_offset + j] = glm::make_vec3(&mesh.positions[j * 3]);
    
            bounding_box_min = glm::min(bounding_box_min, positions[vertex_offset + j]);
            bounding_box_max = glm::max(bounding_box_max, positions[vertex_offset + j]);
        }
        
        const bool has_texcoords = mesh.texcoords.size() / 2 == vertex_count;
        if (has_texcoords)
        {
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                texcoords[vertex_offset + j] = glm::make_vec2(&mesh.texcoords[j * 2]);
                texcoords[vertex_offset + j].y = 1.0f - texcoords[vertex_offset + j].y; // Flip Y
            }
        }
        else
        {
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                texcoords[vertex_offset + j] = glm::vec2(0.0f, 0.0f);
            }
        }
    
        const bool has_normals = mesh.normals.size() / 3 == vertex_count;
        if (has_normals)
        {
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                normals[vertex_offset + j] = glm::make_vec3(&mesh.normals[j * 3]);
            }
        }
        else
        {
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                normals[vertex_offset + j] = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            for (uint32_t j = 0; j < triangle_count; ++j)
            {
                uint32_t index_a = vertex_offset + mesh.indices[j * 3 + 0];
                uint32_t index_b = vertex_offset + mesh.indices[j * 3 + 1];
                uint32_t index_c = vertex_offset + mesh.indices[j * 3 + 2];
    
                glm::vec3 pos_a = positions[index_a];
                glm::vec3 pos_b = positions[index_b];
                glm::vec3 pos_c = positions[index_c];
    
                glm::vec3 normal = glm::normalize(glm::cross(pos_b - pos_a, pos_c - pos_a));
    
                normals[index_a] += normal;
                normals[index_b] += normal;
                normals[index_c] += normal;
            }
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                normals[vertex_offset + j] = glm::normalize(normals[vertex_offset + j]);
            }
        }
    
        for (uint32_t j = 0; j < triangle_count; ++j)
        {
            indices[(triangle_offset + j) * 3 + 0] = mesh.indices[j * 3 + 0];
            indices[(triangle_offset + j) * 3 + 1] = mesh.indices[j * 3 + 1];
            indices[(triangle_offset + j) * 3 + 2] = mesh.indices[j * 3 + 2];
        }
    
        vertex_offset += vertex_count;
        triangle_offset += triangle_count;
    }

    HashTable<int32_t> texture_index_table(256);
    Array<String> texture_filepaths;
    size_t texture_filepaths_size = 0;

    for (uint32_t i = 0; i < material_count; ++i)
    {
        const std::string filenames[] =
        {
            materials[i].diffuse_texname,
        };
        for (uint32_t j = 0; j < ARRAY_COUNT(filenames); ++j)
        {
            uint64_t hash = GfxHash(filenames[j].c_str(), filenames[j].size());
            if (texture_index_table.Find(hash) == NULL)
            {
                String filepath("%s%s", material_dir, filenames[j].c_str());
                if (FileExists(filepath.Data()))
                {
                    texture_index_table.Put(hash, static_cast<int32_t>(texture_filepaths.Count()));
                    texture_filepaths.Push(filepath);
                    texture_filepaths_size += sizeof(uint64_t) + filepath.Length() + 1;
                }
                else
                {
                    texture_index_table.Put(hash, -1);
                }
            }
        }
    }
    
    blob.m_Size =
        sizeof(uint64_t) +                              // Checksum
        sizeof(uint32_t) +                              // Mesh count
        sizeof(GfxModel_T::Mesh) * shape_count +        // Meshes
        sizeof(uint32_t) +                              // Material count
        sizeof(GfxModel_T::Material) * material_count + // Materials
        sizeof(uint32_t) +                              // Texture count
        texture_filepaths_size +                        // Texture filepaths
        sizeof(float) * 3 * 2 +                         // Bounding box
        sizeof(float) +                                 // Vertex position scale
        sizeof(uint16_t) * 4 * total_vertex_count +     // Vertex positions
        sizeof(uint16_t) * 2 * total_vertex_count +     // Vertex texture coordinates
        sizeof(uint8_t)  * 4 * total_vertex_count +     // Vertex normals
        sizeof(uint32_t) * 3 * total_triangle_count;    // Indices
    blob.m_Data = Alloc(blob.m_Size);

    WriteStream stream(blob.m_Data, blob.m_Size);
    stream.WriteUint64(GfxHash(static_cast<const char*>(data), size));

    stream.WriteUint32(shape_count);
    for (uint32_t i = 0, vertex_offset = 0, triangle_offset = 0; i < shape_count; ++i)
    {
        const uint32_t vertex_count = static_cast<uint32_t>(shapes[i].mesh.positions.size() / 3);
        const uint32_t triangle_count = static_cast<uint32_t>(shapes[i].mesh.indices.size() / 3);

        stream.WriteUint32(shapes[i].mesh.material_ids.empty() ? 0xffffffff : shapes[i].mesh.material_ids[0]);
        stream.WriteUint32(vertex_offset);
        stream.WriteUint32(vertex_count);
        stream.WriteUint32(triangle_offset * 3);
        stream.WriteUint32(triangle_count * 3);
    
        vertex_offset += vertex_count;
        triangle_offset += triangle_count;
    }
    
    stream.WriteUint32(material_count);
    for (uint32_t i = 0; i < material_count; ++i)
    {
        stream.WriteInt32(*texture_index_table.Find(GfxHash(materials[i].diffuse_texname.c_str(), materials[i].diffuse_texname.size())));
    }


    stream.WriteUint32(texture_filepaths.Count());
    for (uint32_t i = 0; i < texture_filepaths.Count(); ++i)
    {
        stream.Write(texture_filepaths[i].Data(), texture_filepaths[i].Length() + 1);
    }

    stream.WriteFloat3(&bounding_box_min.x);
    stream.WriteFloat3(&bounding_box_max.x);

    // Find the absolute maximum value of all floats to get the quantization scale
    float q = 1.f / 65535.f;
    for (uint32_t i = 0; i < total_vertex_count; ++i)
    {
        float r = positions[i].x;
        float g = positions[i].y;
        float b = positions[i].z;

        q = fmaxf(q, fabsf(r));
        q = fmaxf(q, fabsf(g));
        q = fmaxf(q, fabsf(b));
    }

    // Store the quantization scale
    stream.WriteFloat(q);

    for (uint32_t i = 0; i < total_vertex_count; ++i)
    {
        float r = positions[i].x;
        float g = positions[i].y;
        float b = positions[i].z;

        // Divide by the quantization scale, now within [-1, 1]
        r /= q;
        g /= q;
        b /= q;

        // Find the absolute maximum value of x, y and z to get the magnitude
        float m = fmaxf(fmaxf(1.f / 65535.f, fabsf(r)), fmaxf(fabsf(g), fabsf(b)));

        // Divide by the magnitude, still within [-1, 1]
        r /= m;
        g /= m;
        b /= m;

        // [-1, 1] -> [0, 1]
        r = r * 0.5f + 0.5f;
        g = g * 0.5f + 0.5f;
        b = b * 0.5f + 0.5f;

        // Convert from float scale to integer scale
        r = r * 65535.f + 0.5f;
        g = g * 65535.f + 0.5f;
        b = b * 65535.f + 0.5f;
        m = m * 65535.f + 0.5f;

        stream.WriteUint16(static_cast<uint16_t>(r));
        stream.WriteUint16(static_cast<uint16_t>(g));
        stream.WriteUint16(static_cast<uint16_t>(b));
        stream.WriteUint16(static_cast<uint16_t>(m));
    }
    
    for (uint32_t i = 0; i < total_vertex_count; ++i)
    {
        stream.WriteUint32(glm::packHalf2x16(texcoords[i]));
    }

    for (uint32_t i = 0; i < total_vertex_count; ++i)
    {
        float r = normals[i].x;
        float g = normals[i].y;
        float b = normals[i].z;
        float a = 0.f;

        // [-1, 1] -> [0, 1]
        r = r * 0.5f + 0.5f;
        g = g * 0.5f + 0.5f;
        b = b * 0.5f + 0.5f;
        a = a * 0.5f + 0.5f;

        // Convert from float scale to integer scale
        r = r * 255.f + 0.5f;
        g = g * 255.f + 0.5f;
        b = b * 255.f + 0.5f;
        a = a * 255.f + 0.5f;

        stream.WriteUint8(static_cast<uint8_t>(r));
        stream.WriteUint8(static_cast<uint8_t>(g));
        stream.WriteUint8(static_cast<uint8_t>(b));
        stream.WriteUint8(static_cast<uint8_t>(a));
    }

    for (uint32_t i = 0; i < total_triangle_count; ++i)
    {
        stream.WriteUint32(indices[i * 3 + 0]);
        stream.WriteUint32(indices[i * 3 + 1]);
        stream.WriteUint32(indices[i * 3 + 2]);
    }

    ASSERT(stream.IsEndOfStream());
    
    Free(positions);
    Free(texcoords);
    Free(normals);
    Free(indices);

    return blob;
}

static GfxModel CreateModel(GfxDevice device, const Blob& blob)
{
    GfxModel model = New<GfxModel_T>();

    ReadStream stream(blob.m_Data, blob.m_Size);
    stream.ReadUint64(); // Checksum

    uint32_t total_vertex_count = 0;
    uint32_t total_index_count = 0;

    model->m_Meshes.Resize(stream.ReadUint32());
    for (uint32_t i = 0; i < model->m_Meshes.Count(); ++i)
    {
        model->m_Meshes[i].m_MaterialIndex = stream.ReadUint32();
        model->m_Meshes[i].m_VertexOffset = stream.ReadUint32();
        model->m_Meshes[i].m_VertexCount = stream.ReadUint32();
        model->m_Meshes[i].m_IndexOffset = stream.ReadUint32();
        model->m_Meshes[i].m_IndexCount = stream.ReadUint32();

        total_vertex_count += model->m_Meshes[i].m_VertexCount;
        total_index_count += model->m_Meshes[i].m_IndexCount;
    }

    model->m_Materials.Resize(stream.ReadUint32());
    for (uint32_t i = 0; i < model->m_Materials.Count(); ++i)
    {
        model->m_Materials[i].m_DiffuseTextureIndex = stream.ReadInt32();
    }

    model->m_Textures.Resize(stream.ReadUint32());
    for (uint32_t i = 0; i < model->m_Textures.Count(); ++i)
    {
        const char* texture_filepath = static_cast<const char*>(stream.Read());
        model->m_Textures[i] = GfxLoadTexture(device, texture_filepath);
    }

    memcpy(model->m_BoundingBoxMin, stream.ReadFloat3(), sizeof(float) * 3);
    memcpy(model->m_BoundingBoxMax, stream.ReadFloat3(), sizeof(float) * 3);

    model->m_QuantizationScale = stream.ReadFloat();

    uint32_t vertex_buffer_size = 0;
    for (uint32_t attribute = 0; attribute < GFX_MODEL_VERTEX_ATTRIBUTE_COUNT; ++attribute)
    {
        model->m_VertexBufferOffsets[attribute] = vertex_buffer_size;
        switch (attribute)
        {
            case GFX_MODEL_VERTEX_ATTRIBUTE_POSITION:
                vertex_buffer_size += sizeof(uint16_t) * 4 * total_vertex_count;
                break;
            case GFX_MODEL_VERTEX_ATTRIBUTE_TEXCOORD:
                vertex_buffer_size += sizeof(uint16_t) * 2 * total_vertex_count;
                break;
            case GFX_MODEL_VERTEX_ATTRIBUTE_NORMAL:
                vertex_buffer_size += sizeof(uint8_t) * 4 * total_vertex_count;
                break;
        }
    }

    GfxCreateBufferParams vertex_buffer_params;
    vertex_buffer_params.m_Usage = GFX_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertex_buffer_params.m_Size = vertex_buffer_size;
    vertex_buffer_params.m_Data = stream.GetPtr();
    model->m_VertexBuffer = GfxCreateBuffer(device, vertex_buffer_params);

    stream.IncrPtr(vertex_buffer_params.m_Size);

    GfxCreateBufferParams index_buffer_params;
    index_buffer_params.m_Usage = GFX_BUFFER_USAGE_INDEX_BUFFER_BIT;
    index_buffer_params.m_Size = sizeof(uint32_t) * total_index_count;
    index_buffer_params.m_Data = stream.GetPtr();
    model->m_IndexBuffer = GfxCreateBuffer(device, index_buffer_params);

    stream.IncrPtr(index_buffer_params.m_Size);

    ASSERT(stream.IsEndOfStream());

    return model;
}

void GfxDestroyModel(GfxDevice device, GfxModel model)
{
    for (uint32_t i = 0; i < model->m_Textures.Count(); ++i)
        GfxDestroyTexture(device, model->m_Textures[i]);
    GfxDestroyBuffer(device, model->m_VertexBuffer);
    GfxDestroyBuffer(device, model->m_IndexBuffer);
    Delete<GfxModel_T>(model);
}

GfxModel GfxLoadModel(GfxDevice device, const char* filepath, const char* material_dir)
{
    const size_t filepath_len = strlen(filepath);

    size_t blob_filepath_offset = 0;
    while (blob_filepath_offset < filepath_len &&
        (filepath[blob_filepath_offset] == '.' ||
         filepath[blob_filepath_offset] == '/' ||
         filepath[blob_filepath_offset] == '\\'))
    {
        ++blob_filepath_offset;
    }
    String blob_filepath("Data/%s.blob", filepath + blob_filepath_offset);

    void* model_data = NULL;
    void* blob_data = NULL;
    size_t model_size = 0;
    size_t blob_size = 0;
    bool model_loaded = ReadFile(filepath, "r", &model_data, &model_size);
    bool blob_loaded = ReadFile(blob_filepath.Data(), "rb", &blob_data, &blob_size);
    if (!model_loaded && !blob_loaded)
    {
        Print("Error: Failed to read from file %s", filepath);
        return NULL;
    }

    bool create_new_blob = false;
    if (model_loaded && blob_loaded)
    {
        uint64_t mesh_checksum = GfxHash(static_cast<const char*>(model_data), model_size);
        uint64_t blob_checksum = *static_cast<const uint64_t*>(blob_data);
        create_new_blob = mesh_checksum != blob_checksum;
    }
    else if (model_loaded)
    {
        create_new_blob = true;
    }

    Blob blob;
    if (create_new_blob)
    {
        blob = CreateModelBlob(model_data, model_size, material_dir ? material_dir : "");
        if (!blob.m_Data || !blob.m_Size)
        {
            if (model_loaded)
                Free(model_data);
            if (blob_loaded)
                Free(blob_data);
            return NULL;
        }
        if (!WriteFile(blob_filepath.Data(), "wb", blob.m_Data, blob.m_Size))
        {
            Print("Error: Failed to write to file %s", blob_filepath.Data());
        }
    }
    else
    {
        blob.m_Data = blob_data;
        blob.m_Size = blob_size;
    }

    GfxModel model = CreateModel(device, blob);

    if (create_new_blob)
        DestroyBlob(blob);
    if (model_loaded)
        Free(model_data);
    if (blob_loaded)
        Free(blob_data);

    Print("Loaded %s", create_new_blob ? filepath : blob_filepath.Data());

    return model;
}

uint32_t GfxGetModelMeshCount(GfxModel model)
{
    return model->m_Meshes.Count();
}
uint32_t GfxGetModelMaterialIndex(GfxModel model, uint32_t mesh_index)
{
    return model->m_Meshes[mesh_index].m_MaterialIndex;
}
GfxTexture GfxGetModelDiffuseTexture(GfxModel model, uint32_t material_index)
{
    int32_t texture_index = model->m_Materials[material_index].m_DiffuseTextureIndex;
    return texture_index == -1 ? NULL : model->m_Textures[texture_index];
}
const float* GfxGetModelBoundingBoxMin(GfxModel model)
{
    return model->m_BoundingBoxMin;
}
const float* GfxGetModelBoundingBoxMax(GfxModel model)
{
    return model->m_BoundingBoxMax;
}
float GfxGetModelQuantizationScale(GfxModel model)
{
    return model->m_QuantizationScale;
}

void GfxCmdBindModelVertexBuffer(GfxCommandBuffer cmd, GfxModel model, GfxModelVertexAttribute attribute, uint32_t binding)
{
    GfxCmdBindVertexBuffer(cmd, binding, model->m_VertexBuffer, model->m_VertexBufferOffsets[attribute]);
}
void GfxCmdBindModelIndexBuffer(GfxCommandBuffer cmd, GfxModel model)
{
    GfxCmdBindIndexBuffer(cmd, model->m_IndexBuffer, 0, sizeof(uint32_t));
}
void GfxCmdDrawModel(GfxCommandBuffer cmd, GfxModel model, uint32_t mesh_index, uint32_t instance_count)
{
    GfxCmdDrawIndexed(cmd, model->m_Meshes[mesh_index].m_IndexCount, instance_count, model->m_Meshes[mesh_index].m_IndexOffset, model->m_Meshes[mesh_index].m_VertexOffset);
}
