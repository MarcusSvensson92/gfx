#include "GfxInternal.h"

#include <json.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

static bool CompileShader(const char* src, const char* stage, void** out_data, size_t* out_size)
{
    char src_filepath[L_tmpnam];
    char dst_filepath[L_tmpnam];
    tmpnam(src_filepath);
    tmpnam(dst_filepath);

    if (!WriteFile(src_filepath, "w", src, strlen(src)))
        return false;

#ifdef _WIN32
    const char* executable_prefix = "";
#else
    const char* executable_prefix = "./";
#endif
    String command("%sglslangValidator -V -S %s -o %s %s", executable_prefix, stage, dst_filepath, src_filepath);
    
    FILE* stream = popen(command.Data(), "r");
    if (!stream)
        return false;

    char stream_buf[2048];
    while (fgets(stream_buf, sizeof(stream_buf), stream))
    {
        char* error = strstr(stream_buf, "ERROR");
        if (error)
        {
            char* line_nr_beg = error + strlen("ERROR: ") + strlen(src_filepath) + strlen(":");
            char* line_nr_end = strchr(line_nr_beg, ':');
            *line_nr_end = '\0';

            int line_nr = atoi(line_nr_beg) - 1; // Line numbering starts at 1

            char* msg_beg = line_nr_end + strlen(": ");
            char* msg_end = strchr(msg_beg, '\n');
            *msg_end = '\0';

            Print("ERROR: %s\n", msg_beg);

            const char* line = src;
            for (int i = 0; i < line_nr + 3; ++i)
            {
                const char* new_line = strchr(line, '\n');
                if (!new_line)
                    break;

                if (i > line_nr - 3)
                {
                    size_t line_len = new_line - line;
                    char line_buf[2048];
                    memcpy(line_buf, line, line_len);
                    line_buf[line_len] = '\0';
                    Print(">%s", line_buf);
                }

                line = new_line + 1;
            }

            Print("");

            break;
        }
    }

    pclose(stream);

    remove(src_filepath);

    if (!ReadFile(dst_filepath, "rb", out_data, out_size))
        return false;

    remove(dst_filepath);

    return true;
}

static Blob CreateTechniqueBlob(const void* json_data, size_t json_size)
{
    #define VERIFY(cond) if (!(cond)) { Print("Error: %s", #cond); free(root); return blob; }

    Blob blob;
    blob.m_Data = NULL;
    blob.m_Size = 0;

    json_parse_result_s result;
    json_value_s* root = json_parse_ex(json_data, json_size, json_parse_flags_allow_json5, 0, 0, &result);
    if (root == NULL)
    {
        Print("JSON parse error %zd: Line %zd Row %zd\n", result.error, result.error_line_no, result.error_row_no);
        return blob;
    }
    VERIFY(root->type == json_type_object);
    json_object_element_s* root_elem = static_cast<json_object_s*>(root->payload)->start;

    bool has_vertex_shader = false;
    bool has_compute_shader = false;

    json_object_element_s* curr_elem = root_elem;
    while (curr_elem)
    {
        if (strcmp(curr_elem->name->string, "vertex_shader") == 0)
            has_vertex_shader = true;
        else if (strcmp(curr_elem->name->string, "compute_shader") == 0)
            has_compute_shader = true;
        curr_elem = curr_elem->next;
    }
    VERIFY(has_vertex_shader || has_compute_shader);
    VERIFY(!(has_vertex_shader && has_compute_shader));

    if (has_vertex_shader)
    {
        GfxGraphicsTechniqueBlob_T graphics_blob;
        graphics_blob.m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        String vs_include, vs_input, vs_main;
        String fs_include, fs_input, fs_main;

        curr_elem = root_elem;
        while (curr_elem)
        {
            if (strcmp(curr_elem->name->string, "vertex_shader") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* vs_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (vs_elem)
                {
                    if (strcmp(vs_elem->name->string, "outputs") == 0)
                    {
                        VERIFY(vs_elem->value->type == json_type_array);
                        json_array_s* outputs = static_cast<json_array_s*>(vs_elem->value->payload);
                        uint32_t i = 0;
                        for (json_array_element_s* output = outputs->start; output != NULL; output = output->next, ++i)
                        {
                            VERIFY(output->value->type == json_type_object);
                            json_object_element_s* output_elem = static_cast<json_object_s*>(output->value->payload)->start;
                            const char* name = NULL;
                            const char* type = NULL;
                            while (output_elem)
                            {
                                if (strcmp(output_elem->name->string, "name") == 0)
                                {
                                    VERIFY(output_elem->value->type == json_type_string);
                                    json_string_s* output_elem_str = static_cast<json_string_s*>(output_elem->value->payload);
                                    name = output_elem_str->string;
                                }
                                else if (strcmp(output_elem->name->string, "type") == 0)
                                {
                                    VERIFY(output_elem->value->type == json_type_string);
                                    json_string_s* output_elem_str = static_cast<json_string_s*>(output_elem->value->payload);
                                    type = output_elem_str->string;
                                }
                                output_elem = output_elem->next;
                            }
                            VERIFY(name && type);
                            vs_include.AppendFormat("layout(location = %u) out %s %s;\n", i, type, name);
                            fs_include.AppendFormat("layout(location = %u) in %s %s;\n", i, type, name);
                        }
                    }
                    else if (strcmp(vs_elem->name->string, "include") == 0)
                    {
                        VERIFY(vs_elem->value->type == json_type_string);
                        json_string_s* vs_str = static_cast<json_string_s*>(vs_elem->value->payload);
                        vs_include.Append(vs_str->string);
                    }
                    else if (strcmp(vs_elem->name->string, "main") == 0)
                    {
                        VERIFY(vs_elem->value->type == json_type_string);
                        json_string_s* vs_str = static_cast<json_string_s*>(vs_elem->value->payload);
                        vs_main.Append(vs_str->string);
                    }
                    vs_elem = vs_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "fragment_shader") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* fs_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (fs_elem)
                {
                    if (strcmp(fs_elem->name->string, "outputs") == 0)
                    {
                        VERIFY(fs_elem->value->type == json_type_array);
                        json_array_s* outputs = static_cast<json_array_s*>(fs_elem->value->payload);
                        uint32_t i = 0;
                        for (json_array_element_s* output = outputs->start; output != NULL; output = output->next, ++i)
                        {
                            VERIFY(output->value->type == json_type_object);
                            json_object_element_s* output_elem = static_cast<json_object_s*>(output->value->payload)->start;
                            const char* name = NULL;
                            const char* type = NULL;
                            while (output_elem)
                            {
                                if (strcmp(output_elem->name->string, "name") == 0)
                                {
                                    VERIFY(output_elem->value->type == json_type_string);
                                    json_string_s* output_elem_str = static_cast<json_string_s*>(output_elem->value->payload);
                                    name = output_elem_str->string;
                                }
                                else if (strcmp(output_elem->name->string, "type") == 0)
                                {
                                    VERIFY(output_elem->value->type == json_type_string);
                                    json_string_s* output_elem_str = static_cast<json_string_s*>(output_elem->value->payload);
                                    type = output_elem_str->string;
                                }
                                output_elem = output_elem->next;
                            }
                            VERIFY(name && type);
                            fs_include.AppendFormat("layout(location = %u) out %s %s;\n", i, type, name);
                        }
                    }
                    else if (strcmp(fs_elem->name->string, "include") == 0)
                    {
                        VERIFY(fs_elem->value->type == json_type_string);
                        json_string_s* fs_str = static_cast<json_string_s*>(fs_elem->value->payload);
                        fs_include.Append(fs_str->string);
                    }
                    else if (strcmp(fs_elem->name->string, "main") == 0)
                    {
                        VERIFY(fs_elem->value->type == json_type_string);
                        json_string_s* fs_str = static_cast<json_string_s*>(fs_elem->value->payload);
                        fs_main.Append(fs_str->string);
                    }
                    fs_elem = fs_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "vertex_attributes") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_array);
                json_array_s* attributes = static_cast<json_array_s*>(curr_elem->value->payload);
                VERIFY(attributes->length <= ARRAY_COUNT(GfxGraphicsTechniqueBlob_T::m_VertexAttributes));
                uint32_t i = 0;
                for (json_array_element_s* attribute = attributes->start; attribute != NULL; attribute = attribute->next, ++i)
                {
                    VERIFY(attribute->value->type == json_type_object);
                    json_object_element_s* attribute_elem = static_cast<json_object_s*>(attribute->value->payload)->start;
                    const char* name = NULL;
                    while (attribute_elem)
                    {
                        if (strcmp(attribute_elem->name->string, "name") == 0)
                        {
                            VERIFY(attribute_elem->value->type == json_type_string);
                            json_string_s* attribute_elem_str = static_cast<json_string_s*>(attribute_elem->value->payload);
                            name = attribute_elem_str->string;
                        }
                        else if (strcmp(attribute_elem->name->string, "binding") == 0)
                        {
                            VERIFY(attribute_elem->value->type == json_type_number);
                            json_number_s* attribute_elem_num = static_cast<json_number_s*>(attribute_elem->value->payload);
                            graphics_blob.m_VertexAttributes[i].m_Binding = static_cast<uint32_t>(atoi(attribute_elem_num->number));
                        }
                        else if (strcmp(attribute_elem->name->string, "format") == 0)
                        {
                            VERIFY(attribute_elem->value->type == json_type_string);
                            json_string_s* attribute_elem_str = static_cast<json_string_s*>(attribute_elem->value->payload);
                            graphics_blob.m_VertexAttributes[i].m_Format = ToVkFormat(attribute_elem_str->string);
                            VERIFY(graphics_blob.m_VertexAttributes[i].m_Format != VK_FORMAT_UNDEFINED);
                        }
                        else if (strcmp(attribute_elem->name->string, "offset") == 0)
                        {
                            VERIFY(attribute_elem->value->type == json_type_number);
                            json_number_s* attribute_elem_num = static_cast<json_number_s*>(attribute_elem->value->payload);
                            graphics_blob.m_VertexAttributes[i].m_Offset = static_cast<uint32_t>(atoi(attribute_elem_num->number));
                        }
                        else if (strcmp(attribute_elem->name->string, "input_rate") == 0)
                        {
                            VERIFY(attribute_elem->value->type == json_type_string);
                            json_string_s* attribute_elem_str = static_cast<json_string_s*>(attribute_elem->value->payload);
                            graphics_blob.m_VertexAttributes[i].m_InputRate = ToVkVertexInputRate(attribute_elem_str->string);
                            VERIFY(graphics_blob.m_VertexAttributes[i].m_InputRate != VK_VERTEX_INPUT_RATE_MAX_ENUM);
                        }
                        attribute_elem = attribute_elem->next;
                    }
                    VERIFY(name);
                    VERIFY(graphics_blob.m_VertexAttributes[i].m_Binding != ~0U);
                    VERIFY(graphics_blob.m_VertexAttributes[i].m_Format != VK_FORMAT_UNDEFINED);
                    VERIFY(graphics_blob.m_VertexAttributes[i].m_Offset != ~0U);
                    VERIFY(graphics_blob.m_VertexAttributes[i].m_InputRate != VK_VERTEX_INPUT_RATE_MAX_ENUM);
                    const char* type_string = ToShaderFormatString(graphics_blob.m_VertexAttributes[i].m_Format);
                    vs_include.AppendFormat("layout(location = %u) in %s %s;\n", i, type_string, name);
                }
                graphics_blob.m_VertexAttributeCount = i;
            }
            else if (strcmp(curr_elem->name->string, "shader_bindings") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_array);
                json_array_s* bindings = static_cast<json_array_s*>(curr_elem->value->payload);
                VERIFY(bindings->length <= ARRAY_COUNT(GfxTechniqueBlob_T::m_ShaderBindings));
                uint32_t i = 0;
                for (json_array_element_s* binding = bindings->start; binding != NULL; binding = binding->next, ++i)
                {
                    VERIFY(binding->value->type == json_type_object);
                    json_object_element_s* binding_elem = static_cast<json_object_s*>(binding->value->payload)->start;
                    const char* name = NULL;
                    const char* type = NULL;
                    const char* format = NULL;
                    const char* content = NULL;
                    while (binding_elem)
                    {
                        if (strcmp(binding_elem->name->string, "name") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            name = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "type") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            type = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "format") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            format = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "content") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            content = binding_elem_str->string;
                        }
                        binding_elem = binding_elem->next;
                    }
                    VERIFY(name && type);
                    graphics_blob.m_ShaderBindings[i].m_Hash = GfxHash(name, strlen(name));
                    graphics_blob.m_ShaderBindings[i].m_Type = ToVkDescriptorType(type);
                    VERIFY(graphics_blob.m_ShaderBindings[i].m_Hash != 0);
                    VERIFY(graphics_blob.m_ShaderBindings[i].m_Type != VK_DESCRIPTOR_TYPE_MAX_ENUM);
                    String shader_str;
                    if (format && content)
                        shader_str.AppendFormat("layout(binding = %u, %s) %s %s\n{%s};\n", i, format, ToShaderDescriptorString(type), name, content);
                    else if (format)
                        shader_str.AppendFormat("layout(binding = %u, %s) %s %s;\n", i, format, ToShaderDescriptorString(type), name);
                    else if (content)
                        shader_str.AppendFormat("layout(binding = %u) %s %s\n{%s};\n", i, ToShaderDescriptorString(type), name, content);
                    else
                        shader_str.AppendFormat("layout(binding = %u) %s %s;\n", i, ToShaderDescriptorString(type), name);
                    vs_include.Append(shader_str);
                    fs_include.Append(shader_str);
                }
                graphics_blob.m_ShaderBindingCount = i;
            }
            else if (strcmp(curr_elem->name->string, "color_attachments") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_array);
                json_array_s* formats = static_cast<json_array_s*>(curr_elem->value->payload);
                VERIFY(formats->length <= ARRAY_COUNT(GfxGraphicsTechniqueBlob_T::m_ColorAttachmentFormats));
                uint32_t i = 0;
                for (json_array_element_s* format = formats->start; format != NULL; format = format->next, ++i)
                {
                    VERIFY(format->value->type == json_type_string);
                    json_string_s* format_str = static_cast<json_string_s*>(format->value->payload);
                    graphics_blob.m_ColorAttachmentFormats[i] = strcmp(format_str->string, "back_buffer") == 0 ? VK_FORMAT_RANGE_SIZE : ToVkFormat(format_str->string);
                    VERIFY(graphics_blob.m_ColorAttachmentFormats[i] != VK_FORMAT_UNDEFINED);
                }
                graphics_blob.m_ColorAttachmentCount = i;
            }
            else if (strcmp(curr_elem->name->string, "depth_attachment") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_string);
                json_string_s* format_str = static_cast<json_string_s*>(curr_elem->value->payload);
                graphics_blob.m_DepthAttachmentFormat = ToVkFormat(format_str->string);
                VERIFY(graphics_blob.m_DepthAttachmentFormat != VK_FORMAT_UNDEFINED);
            }
            else if (strcmp(curr_elem->name->string, "input_assembly") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* ia_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (ia_elem)
                {
                    if (strcmp(ia_elem->name->string, "topology") == 0)
                    {
                        VERIFY(ia_elem->value->type == json_type_string);
                        json_string_s* ia_elem_str = static_cast<json_string_s*>(ia_elem->value->payload);
                        graphics_blob.m_InputAssembly.m_Topology = ToVkPrimitiveTopology(ia_elem_str->string);
                        VERIFY(graphics_blob.m_InputAssembly.m_Topology != VK_PRIMITIVE_TOPOLOGY_MAX_ENUM);
                    }
                    else if (strcmp(ia_elem->name->string, "primitive_restart_enable") == 0)
                    {
                        VERIFY(ia_elem->value->type == json_type_true || ia_elem->value->type == json_type_false);
                        graphics_blob.m_InputAssembly.m_PrimitiveRestartEnable = ia_elem->value->type == json_type_true;
                    }
                    ia_elem = ia_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "rasterizer_state") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* rs_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (rs_elem)
                {
                    if (strcmp(rs_elem->name->string, "depth_clamp_enable") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_true || rs_elem->value->type == json_type_false);
                        graphics_blob.m_RasterizerState.m_DepthClampEnable = rs_elem->value->type == json_type_true;
                    }
                    else if (strcmp(rs_elem->name->string, "rasterizer_discard_enable") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_true || rs_elem->value->type == json_type_false);
                        graphics_blob.m_RasterizerState.m_RasterizerDiscardEnable = rs_elem->value->type == json_type_true;
                    }
                    else if (strcmp(rs_elem->name->string, "polygon_mode") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_string);
                        json_string_s* rs_elem_str = static_cast<json_string_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_PolygonMode = ToVkPolygonMode(rs_elem_str->string);
                        VERIFY(graphics_blob.m_RasterizerState.m_PolygonMode != VK_POLYGON_MODE_MAX_ENUM);
                    }
                    else if (strcmp(rs_elem->name->string, "cull_mode") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_string);
                        json_string_s* rs_elem_str = static_cast<json_string_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_CullMode = ToVkCullMode(rs_elem_str->string);
                        VERIFY(graphics_blob.m_RasterizerState.m_CullMode != VK_CULL_MODE_FLAG_BITS_MAX_ENUM);
                    }
                    else if (strcmp(rs_elem->name->string, "front_face") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_string);
                        json_string_s* rs_elem_str = static_cast<json_string_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_FrontFace = ToVkFrontFace(rs_elem_str->string);
                        VERIFY(graphics_blob.m_RasterizerState.m_FrontFace != VK_FRONT_FACE_MAX_ENUM);
                    }
                    else if (strcmp(rs_elem->name->string, "depth_bias_enable") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_true || rs_elem->value->type == json_type_false);
                        graphics_blob.m_RasterizerState.m_DepthBiasEnable = rs_elem->value->type == json_type_true;
                    }
                    else if (strcmp(rs_elem->name->string, "depth_bias_constant_factor") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_number);
                        json_number_s* rs_elem_num = static_cast<json_number_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_DepthBiasConstantFactor = static_cast<float>(atof(rs_elem_num->number));
                    }
                    else if (strcmp(rs_elem->name->string, "depth_bias_clamp") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_number);
                        json_number_s* rs_elem_num = static_cast<json_number_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_DepthBiasClamp = static_cast<float>(atof(rs_elem_num->number));
                    }
                    else if (strcmp(rs_elem->name->string, "depth_bias_slope_factor") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_number);
                        json_number_s* rs_elem_num = static_cast<json_number_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_DepthBiasSlopeFactor = static_cast<float>(atof(rs_elem_num->number));
                    }
                    else if (strcmp(rs_elem->name->string, "line_width") == 0)
                    {
                        VERIFY(rs_elem->value->type == json_type_number);
                        json_number_s* rs_elem_num = static_cast<json_number_s*>(rs_elem->value->payload);
                        graphics_blob.m_RasterizerState.m_LineWidth = static_cast<float>(atof(rs_elem_num->number));
                    }
                    rs_elem = rs_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "depth_stencil_state") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* ds_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (ds_elem)
                {
                    if (strcmp(ds_elem->name->string, "depth_test_enable") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_true || ds_elem->value->type == json_type_false);
                        graphics_blob.m_DepthStencilState.m_DepthTestEnable = ds_elem->value->type == json_type_true;
                    }
                    else if (strcmp(ds_elem->name->string, "depth_write_enable") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_true || ds_elem->value->type == json_type_false);
                        graphics_blob.m_DepthStencilState.m_DepthWriteEnable = ds_elem->value->type == json_type_true;
                    }
                    else if (strcmp(ds_elem->name->string, "depth_compare_op") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_string);
                        json_string_s* ds_elem_str = static_cast<json_string_s*>(ds_elem->value->payload);
                        graphics_blob.m_DepthStencilState.m_DepthCompareOp = ToVkCompareOp(ds_elem_str->string);
                        VERIFY(graphics_blob.m_DepthStencilState.m_DepthCompareOp != VK_COMPARE_OP_MAX_ENUM);
                    }
                    else if (strcmp(ds_elem->name->string, "depth_bounds_test_enable") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_true || ds_elem->value->type == json_type_false);
                        graphics_blob.m_DepthStencilState.m_DepthBoundsTestEnable = ds_elem->value->type == json_type_true;
                    }
                    else if (strcmp(ds_elem->name->string, "stencil_test_enable") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_true || ds_elem->value->type == json_type_false);
                        graphics_blob.m_DepthStencilState.m_StencilTestEnable = ds_elem->value->type == json_type_true;
                    }
                    else if (strcmp(ds_elem->name->string, "front") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_object);
                        json_object_element_s* front_elem = static_cast<json_object_s*>(ds_elem->value->payload)->start;
                        while (front_elem)
                        {
                            if (strcmp(front_elem->name->string, "fail_op") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_string);
                                json_string_s* front_elem_str = static_cast<json_string_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.failOp = ToVkStencilOp(front_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Front.failOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(front_elem->name->string, "pass_op") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_string);
                                json_string_s* front_elem_str = static_cast<json_string_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.passOp = ToVkStencilOp(front_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Front.passOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(front_elem->name->string, "depth_fail_op") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_string);
                                json_string_s* front_elem_str = static_cast<json_string_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.depthFailOp = ToVkStencilOp(front_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Front.depthFailOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(front_elem->name->string, "compare_op") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_string);
                                json_string_s* front_elem_str = static_cast<json_string_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.compareOp = ToVkCompareOp(front_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Front.compareOp != VK_COMPARE_OP_MAX_ENUM);
                            }
                            else if (strcmp(front_elem->name->string, "compare_mask") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_number);
                                json_number_s* front_elem_num = static_cast<json_number_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.compareMask = static_cast<uint32_t>(atoi(front_elem_num->number));
                            }
                            else if (strcmp(front_elem->name->string, "write_mask") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_number);
                                json_number_s* front_elem_num = static_cast<json_number_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.writeMask = static_cast<uint32_t>(atoi(front_elem_num->number));
                            }
                            else if (strcmp(front_elem->name->string, "reference") == 0)
                            {
                                VERIFY(front_elem->value->type == json_type_number);
                                json_number_s* front_elem_num = static_cast<json_number_s*>(front_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Front.reference = static_cast<uint32_t>(atoi(front_elem_num->number));
                            }
                            front_elem = front_elem->next;
                        }
                    }
                    else if (strcmp(ds_elem->name->string, "back") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_object);
                        json_object_element_s* back_elem = static_cast<json_object_s*>(ds_elem->value->payload)->start;
                        while (back_elem)
                        {
                            if (strcmp(back_elem->name->string, "fail_op") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_string);
                                json_string_s* back_elem_str = static_cast<json_string_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.failOp = ToVkStencilOp(back_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Back.failOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(back_elem->name->string, "pass_op") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_string);
                                json_string_s* back_elem_str = static_cast<json_string_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.passOp = ToVkStencilOp(back_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Back.passOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(back_elem->name->string, "depth_fail_op") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_string);
                                json_string_s* back_elem_str = static_cast<json_string_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.depthFailOp = ToVkStencilOp(back_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Back.depthFailOp != VK_STENCIL_OP_MAX_ENUM);
                            }
                            else if (strcmp(back_elem->name->string, "compare_op") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_string);
                                json_string_s* back_elem_str = static_cast<json_string_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.compareOp = ToVkCompareOp(back_elem_str->string);
                                VERIFY(graphics_blob.m_DepthStencilState.m_Back.compareOp != VK_COMPARE_OP_MAX_ENUM);
                            }
                            else if (strcmp(back_elem->name->string, "compare_mask") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_number);
                                json_number_s* back_elem_num = static_cast<json_number_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.compareMask = static_cast<uint32_t>(atoi(back_elem_num->number));
                            }
                            else if (strcmp(back_elem->name->string, "write_mask") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_number);
                                json_number_s* back_elem_num = static_cast<json_number_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.writeMask = static_cast<uint32_t>(atoi(back_elem_num->number));
                            }
                            else if (strcmp(back_elem->name->string, "reference") == 0)
                            {
                                VERIFY(back_elem->value->type == json_type_number);
                                json_number_s* back_elem_num = static_cast<json_number_s*>(back_elem->value->payload);
                                graphics_blob.m_DepthStencilState.m_Back.reference = static_cast<uint32_t>(atoi(back_elem_num->number));
                            }
                            back_elem = back_elem->next;
                        }
                    }
                    else if (strcmp(ds_elem->name->string, "min_depth_bounds") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_number);
                        json_number_s* ds_elem_num = static_cast<json_number_s*>(ds_elem->value->payload);
                        graphics_blob.m_DepthStencilState.m_MinDepthBounds = static_cast<float>(atof(ds_elem_num->number));
                    }
                    else if (strcmp(ds_elem->name->string, "max_depth_bounds") == 0)
                    {
                        VERIFY(ds_elem->value->type == json_type_number);
                        json_number_s* ds_elem_num = static_cast<json_number_s*>(ds_elem->value->payload);
                        graphics_blob.m_DepthStencilState.m_MaxDepthBounds = static_cast<float>(atof(ds_elem_num->number));
                    }
                    ds_elem = ds_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "blend_attachments") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_array);
                json_array_s* states = static_cast<json_array_s*>(curr_elem->value->payload);
                VERIFY(states->length <= ARRAY_COUNT(GfxGraphicsTechniqueBlob_T::m_BlendAttachments));
                uint32_t i = 0;
                for (json_array_element_s* state = states->start; state != NULL; state = state->next, ++i)
                {
                    VERIFY(state->value->type == json_type_object);
                    json_object_element_s* state_elem = static_cast<json_object_s*>(state->value->payload)->start;
                    while (state_elem)
                    {
                        if (strcmp(state_elem->name->string, "blend_enable") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_true || state_elem->value->type == json_type_false);
                            graphics_blob.m_BlendAttachments[i].m_BlendEnable = state_elem->value->type == json_type_true;
                        }
                        else if (strcmp(state_elem->name->string, "src_color_blend_factor") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_SrcColorBlendFactor = ToVkBlendFactor(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_SrcColorBlendFactor != VK_BLEND_FACTOR_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "dst_color_blend_factor") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_DstColorBlendFactor = ToVkBlendFactor(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_DstColorBlendFactor != VK_BLEND_FACTOR_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "color_blend_op") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_ColorBlendOp = ToVkBlendOp(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_ColorBlendOp != VK_BLEND_OP_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "src_alpha_blend_factor") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_SrcAlphaBlendFactor = ToVkBlendFactor(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_SrcAlphaBlendFactor != VK_BLEND_FACTOR_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "dst_alpha_blend_factor") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_DstAlphaBlendFactor = ToVkBlendFactor(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_DstAlphaBlendFactor != VK_BLEND_FACTOR_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "alpha_blend_op") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_AlphaBlendOp = ToVkBlendOp(state_elem_str->string);
                            VERIFY(graphics_blob.m_BlendAttachments[i].m_AlphaBlendOp != VK_BLEND_OP_MAX_ENUM);
                        }
                        else if (strcmp(state_elem->name->string, "color_write_mask") == 0)
                        {
                            VERIFY(state_elem->value->type == json_type_string);
                            json_string_s* state_elem_str = static_cast<json_string_s*>(state_elem->value->payload);
                            graphics_blob.m_BlendAttachments[i].m_ColorWriteMask = ToVkColorComponentMask(state_elem_str->string);
                        }
                        state_elem = state_elem->next;
                    }
                }
            }
            curr_elem = curr_elem->next;
        }

        void* vs_code = NULL;
        void* fs_code = NULL;
        size_t vs_size = 0;
        size_t fs_size = 0;

        String vs_src;
        vs_src.Append("#version 450\n#extension GL_ARB_separate_shader_objects : enable\n#extension GL_ARB_shading_language_packing : enable\n");
        vs_src.Append(vs_include);
        vs_src.Append("out gl_PerVertex { vec4 gl_Position; };\nvoid main()\n{");
        vs_src.Append(vs_main);
        vs_src.Append("}");
        if (!CompileShader(vs_src.Data(), "vert", &vs_code, &vs_size))
        {
            free(root);
            return blob;
        }

        if (fs_main.Length())
        {
            String fs_src;
            fs_src.Append("#version 450\n#extension GL_ARB_separate_shader_objects : enable\n#extension GL_ARB_shading_language_packing : enable\n");
            fs_src.Append(fs_include);
            fs_src.Append("void main()\n{");
            fs_src.Append(fs_main);
            fs_src.Append("}");
            if (!CompileShader(fs_src.Data(), "frag", &fs_code, &fs_size))
            {
                free(root);
                return blob;
            }
        }

        blob.m_Size =
            sizeof(uint64_t) +                                      // Checksum
            sizeof(uint64_t) + sizeof(GfxGraphicsTechniqueBlob_T) + // Main blob
            sizeof(uint64_t) + vs_size +                            // Vertex shader
            sizeof(uint64_t) + fs_size;                             // Fragment shader
        blob.m_Data = Alloc(blob.m_Size);

        WriteStream stream(blob.m_Data, blob.m_Size);
        stream.WriteUint64(GfxHash(static_cast<const char*>(json_data), json_size));
        stream.Write(&graphics_blob, sizeof(GfxGraphicsTechniqueBlob_T));
        stream.Write(vs_code, vs_size);
        stream.Write(fs_code, fs_size);
        ASSERT(stream.IsEndOfStream());

        Free(vs_code);
        if (fs_code)
            Free(fs_code);
    }
    else
    {
        GfxTechniqueBlob_T compute_blob;
        compute_blob.m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        String cs_include, cs_main;

        curr_elem = root_elem;
        while (curr_elem)
        {
            if (strcmp(curr_elem->name->string, "compute_shader") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_object);
                json_object_element_s* cs_elem = static_cast<json_object_s*>(curr_elem->value->payload)->start;
                while (cs_elem)
                {
                    if (strcmp(cs_elem->name->string, "work_group_size") == 0)
                    {
                        VERIFY(cs_elem->value->type == json_type_object);
                        json_object_element_s* size_elem = static_cast<json_object_s*>(cs_elem->value->payload)->start;
                        uint32_t x = 0, y = 0, z = 0;
                        while (size_elem)
                        {
                            if (strcmp(size_elem->name->string, "x") == 0)
                            {
                                VERIFY(size_elem->value->type == json_type_number);
                                json_number_s* size_elem_num = static_cast<json_number_s*>(size_elem->value->payload);
                                x = static_cast<uint32_t>(atoi(size_elem_num->number));
                            }
                            else if (strcmp(size_elem->name->string, "y") == 0)
                            {
                                VERIFY(size_elem->value->type == json_type_number);
                                json_number_s* size_elem_num = static_cast<json_number_s*>(size_elem->value->payload);
                                y = static_cast<uint32_t>(atoi(size_elem_num->number));
                            }
                            else if (strcmp(size_elem->name->string, "z") == 0)
                            {
                                VERIFY(size_elem->value->type == json_type_number);
                                json_number_s* size_elem_num = static_cast<json_number_s*>(size_elem->value->payload);
                                z = static_cast<uint32_t>(atoi(size_elem_num->number));
                            }
                            size_elem = size_elem->next;
                        }
                        cs_include.AppendFormat("layout (local_size_x = %u, local_size_y = %u, local_size_z = %u) in;\n", x, y, z);
                    }
                    else if (strcmp(cs_elem->name->string, "include") == 0)
                    {
                        VERIFY(cs_elem->value->type == json_type_string);
                        json_string_s* cs_str = static_cast<json_string_s*>(cs_elem->value->payload);
                        cs_include.Append(cs_str->string);
                    }
                    else if (strcmp(cs_elem->name->string, "main") == 0)
                    {
                        VERIFY(cs_elem->value->type == json_type_string);
                        json_string_s* cs_str = static_cast<json_string_s*>(cs_elem->value->payload);
                        cs_main.Append(cs_str->string);
                    }
                    cs_elem = cs_elem->next;
                }
            }
            else if (strcmp(curr_elem->name->string, "shader_bindings") == 0)
            {
                VERIFY(curr_elem->value->type == json_type_array);
                json_array_s* bindings = static_cast<json_array_s*>(curr_elem->value->payload);
                VERIFY(bindings->length <= ARRAY_COUNT(GfxTechniqueBlob_T::m_ShaderBindings));
                uint32_t i = 0;
                for (json_array_element_s* binding = bindings->start; binding != NULL; binding = binding->next, ++i)
                {
                    VERIFY(binding->value->type == json_type_object);
                    json_object_element_s* binding_elem = static_cast<json_object_s*>(binding->value->payload)->start;
                    const char* name = NULL;
                    const char* type = NULL;
                    const char* format = NULL;
                    const char* content = NULL;
                    while (binding_elem)
                    {
                        if (strcmp(binding_elem->name->string, "name") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            name = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "type") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            type = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "format") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            format = binding_elem_str->string;
                        }
                        else if (strcmp(binding_elem->name->string, "content") == 0)
                        {
                            VERIFY(binding_elem->value->type == json_type_string);
                            json_string_s* binding_elem_str = static_cast<json_string_s*>(binding_elem->value->payload);
                            content = binding_elem_str->string;
                        }
                        binding_elem = binding_elem->next;
                    }
                    VERIFY(name && type);
                    compute_blob.m_ShaderBindings[i].m_Hash = GfxHash(name, strlen(name));
                    compute_blob.m_ShaderBindings[i].m_Type = ToVkDescriptorType(type);
                    VERIFY(compute_blob.m_ShaderBindings[i].m_Hash != 0);
                    VERIFY(compute_blob.m_ShaderBindings[i].m_Type != VK_DESCRIPTOR_TYPE_MAX_ENUM);
                    String shader_str;
                    if (format && content)
                        shader_str.AppendFormat("layout(binding = %u, %s) %s %s {%s};\n", i, format, ToShaderDescriptorString(type), name, content);
                    else if (format)
                        shader_str.AppendFormat("layout(binding = %u, %s) %s %s;\n", i, format, ToShaderDescriptorString(type), name);
                    else if (content)
                        shader_str.AppendFormat("layout(binding = %u) %s %s {%s};\n", i, ToShaderDescriptorString(type), name, content);
                    else
                        shader_str.AppendFormat("layout(binding = %u) %s %s;\n", i, ToShaderDescriptorString(type), name);
                    cs_include.Append(shader_str);
                }
                compute_blob.m_ShaderBindingCount = i;
            }
            curr_elem = curr_elem->next;
        }

        void* cs_code = NULL;
        size_t cs_size = 0;

        String cs_src;
        cs_src.Append("#version 450\n#extension GL_ARB_separate_shader_objects : enable\n#extension GL_ARB_shading_language_packing : enable\n");
        cs_src.Append(cs_include);
        cs_src.Append("void main()\n{");
        cs_src.Append(cs_main);
        cs_src.Append("}");
        if (!CompileShader(cs_src.Data(), "comp", &cs_code, &cs_size))
        {
            free(root);
            return blob;
        }

        blob.m_Size =
            sizeof(uint64_t) +                              // Checksum
            sizeof(size_t) + sizeof(GfxTechniqueBlob_T) +   // Main blob
            sizeof(size_t) + cs_size;                       // Compute shader
        blob.m_Data = Alloc(blob.m_Size);

        WriteStream stream(blob.m_Data, blob.m_Size);
        stream.WriteUint64(GfxHash(static_cast<const char*>(json_data), json_size));
        stream.Write(&compute_blob, sizeof(GfxTechniqueBlob_T));
        stream.Write(cs_code, cs_size);
        ASSERT(stream.IsEndOfStream());

        Free(cs_code);
    }

    return blob;

#undef VERIFY
}

GfxTechnique GfxCreateTechnique(GfxDevice device, const void* data, size_t size, GfxTechnique old_tech)
{
    ASSERT(data && size);

    ReadStream stream(const_cast<void*>(data), size);
    stream.ReadUint64(); // Checksum

    const GfxTechniqueBlob_T* blob_ptr = static_cast<const GfxTechniqueBlob_T*>(stream.Read());

    GfxTechnique_T* tech = old_tech;
    if (tech)
    {
        vkDestroyPipeline(device->m_Device, tech->m_Pipeline, NULL);
        vkDestroyPipelineLayout(device->m_Device, tech->m_PipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device->m_Device, tech->m_DescriptorSetLayout, NULL);
        if (tech->m_BindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
            vkDestroyRenderPass(device->m_Device, tech->m_RenderPass, NULL);

        tech->m_ShaderBindings.Clear();
    }
    else
    {
        tech = New<GfxTechnique_T>();
    }
    tech->m_BindPoint = blob_ptr->m_BindPoint;

    // Pipeline layout
    {
        tech->m_ShaderBindingCount = blob_ptr->m_ShaderBindingCount;

        Array<VkDescriptorSetLayoutBinding> bindings(blob_ptr->m_ShaderBindingCount);
        for (uint32_t i = 0; i < blob_ptr->m_ShaderBindingCount; ++i)
        {
            bindings[i].binding = i;
            bindings[i].descriptorType = blob_ptr->m_ShaderBindings[i].m_Type;
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = tech->m_BindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_COMPUTE_BIT;
            bindings[i].pImmutableSamplers = NULL;

            GfxTechnique_T::ShaderBinding binding;
            binding.m_Binding = i;
            binding.m_Type = blob_ptr->m_ShaderBindings[i].m_Type;
            tech->m_ShaderBindings.Put(blob_ptr->m_ShaderBindings[i].m_Hash, binding);
        }

        VkDescriptorSetLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = bindings.Count();
        layout_info.pBindings = bindings.Data();
        VK(vkCreateDescriptorSetLayout(device->m_Device, &layout_info, NULL, &tech->m_DescriptorSetLayout));

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &tech->m_DescriptorSetLayout;
        VK(vkCreatePipelineLayout(device->m_Device, &pipeline_layout_info, NULL, &tech->m_PipelineLayout));
    }

    if (tech->m_BindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        const GfxGraphicsTechniqueBlob_T* graphics_blob_ptr = reinterpret_cast<const GfxGraphicsTechniqueBlob_T*>(blob_ptr);

        size_t vs_size, fs_size;
        const uint32_t* vs_code = static_cast<const uint32_t*>(stream.Read(&vs_size));
        const uint32_t* fs_code = static_cast<const uint32_t*>(stream.Read(&fs_size));

        ASSERT(stream.IsEndOfStream());

        // Render pass
        {
            const uint32_t color_attachment_count = graphics_blob_ptr->m_ColorAttachmentCount;
            const uint32_t depth_attachment_count = graphics_blob_ptr->m_DepthAttachmentFormat != VK_FORMAT_UNDEFINED ? 1 : 0;

            Array<VkAttachmentDescription> attachments(color_attachment_count + depth_attachment_count);
            for (uint32_t i = 0; i < graphics_blob_ptr->m_ColorAttachmentCount; ++i)
            {
                attachments[i] = {};
                attachments[i].format = graphics_blob_ptr->m_ColorAttachmentFormats[i] == VK_FORMAT_RANGE_SIZE ? device->m_SwapchainSurfaceFormat.format : graphics_blob_ptr->m_ColorAttachmentFormats[i];
                attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            if (depth_attachment_count)
            {
                attachments[color_attachment_count] = {};
                attachments[color_attachment_count].format = graphics_blob_ptr->m_DepthAttachmentFormat;
                attachments[color_attachment_count].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[color_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachments[color_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[color_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachments[color_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[color_attachment_count].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments[color_attachment_count].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            Array<VkAttachmentReference> color_attachments(color_attachment_count);
            for (uint32_t i = 0; i < graphics_blob_ptr->m_ColorAttachmentCount; ++i)
            {
                color_attachments[i].attachment = i;
                color_attachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            VkAttachmentReference depth_attachment;
            if (graphics_blob_ptr->m_DepthAttachmentFormat != VK_FORMAT_UNDEFINED)
            {
                depth_attachment.attachment = color_attachment_count;
                depth_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            VkSubpassDescription subpass_description = {};
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass_description.colorAttachmentCount = color_attachments.Count();
            subpass_description.pColorAttachments = color_attachments.Data();
            subpass_description.pDepthStencilAttachment = depth_attachment_count ? &depth_attachment : NULL;

            VkRenderPassCreateInfo render_pass_info = {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = attachments.Count();
            render_pass_info.pAttachments = attachments.Data();
            render_pass_info.subpassCount = 1;
            render_pass_info.pSubpasses = &subpass_description;
            VK(vkCreateRenderPass(device->m_Device, &render_pass_info, NULL, &tech->m_RenderPass));
        }

        // Pipeline
        {
            VkShaderModule vs_module = VK_NULL_HANDLE;
            VkShaderModule fs_module = VK_NULL_HANDLE;

            VkShaderModuleCreateInfo vs_module_create_info = {};
            vs_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            vs_module_create_info.codeSize = vs_size;
            vs_module_create_info.pCode = vs_code;
            VK(vkCreateShaderModule(device->m_Device, &vs_module_create_info, NULL, &vs_module));
            if (fs_size)
            {
                VkShaderModuleCreateInfo fs_module_create_info = {};
                fs_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                fs_module_create_info.codeSize = fs_size;
                fs_module_create_info.pCode = fs_code;
                VK(vkCreateShaderModule(device->m_Device, &fs_module_create_info, NULL, &fs_module));
            }

            VkPipelineShaderStageCreateInfo shader_stages[2] = {};
            shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_stages[0].module = vs_module;
            shader_stages[0].pName = "main";
            if (fs_module)
            {
                shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shader_stages[1].module = fs_module;
                shader_stages[1].pName = "main";
            }

            Array<VkVertexInputBindingDescription> vertex_bindings(graphics_blob_ptr->m_VertexAttributeCount);
            for (uint32_t i = 0; i < graphics_blob_ptr->m_VertexAttributeCount; ++i)
            {
                vertex_bindings[i].binding = i;
                vertex_bindings[i].stride = 0;
            }

            uint32_t vertex_binding_count = 0;
            Array<VkVertexInputAttributeDescription> vertex_attributes(graphics_blob_ptr->m_VertexAttributeCount);
            for (uint32_t i = 0; i < graphics_blob_ptr->m_VertexAttributeCount; ++i)
            {
                const uint32_t binding = graphics_blob_ptr->m_VertexAttributes[i].m_Binding;

                vertex_attributes[i].location = i;
                vertex_attributes[i].binding = binding;
                vertex_attributes[i].format = graphics_blob_ptr->m_VertexAttributes[i].m_Format;
                vertex_attributes[i].offset = graphics_blob_ptr->m_VertexAttributes[i].m_Offset;

                vertex_bindings[binding].stride += ToTexelStride(graphics_blob_ptr->m_VertexAttributes[i].m_Format);
                vertex_bindings[binding].inputRate = graphics_blob_ptr->m_VertexAttributes[i].m_InputRate;

                vertex_binding_count = binding + 1 > vertex_binding_count ? binding + 1 : vertex_binding_count;
            }

            VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
            vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state.vertexBindingDescriptionCount = vertex_binding_count;
            vertex_input_state.pVertexBindingDescriptions = vertex_bindings.Data();
            vertex_input_state.vertexAttributeDescriptionCount = vertex_attributes.Count();
            vertex_input_state.pVertexAttributeDescriptions = vertex_attributes.Data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = graphics_blob_ptr->m_InputAssembly.m_Topology;
            input_assembly.primitiveRestartEnable = graphics_blob_ptr->m_InputAssembly.m_PrimitiveRestartEnable;

            VkPipelineRasterizationStateCreateInfo rasterizer_state = {};
            rasterizer_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer_state.depthClampEnable = graphics_blob_ptr->m_RasterizerState.m_DepthClampEnable;
            rasterizer_state.rasterizerDiscardEnable = graphics_blob_ptr->m_RasterizerState.m_RasterizerDiscardEnable;
            rasterizer_state.polygonMode = graphics_blob_ptr->m_RasterizerState.m_PolygonMode;
            rasterizer_state.cullMode = graphics_blob_ptr->m_RasterizerState.m_CullMode;
            rasterizer_state.frontFace = graphics_blob_ptr->m_RasterizerState.m_FrontFace;
            rasterizer_state.depthBiasEnable = graphics_blob_ptr->m_RasterizerState.m_DepthBiasEnable;
            rasterizer_state.depthBiasConstantFactor = graphics_blob_ptr->m_RasterizerState.m_DepthBiasConstantFactor;
            rasterizer_state.depthBiasClamp = graphics_blob_ptr->m_RasterizerState.m_DepthBiasClamp;
            rasterizer_state.depthBiasSlopeFactor = graphics_blob_ptr->m_RasterizerState.m_DepthBiasSlopeFactor;
            rasterizer_state.lineWidth = graphics_blob_ptr->m_RasterizerState.m_LineWidth;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
            depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_state.depthTestEnable = graphics_blob_ptr->m_DepthStencilState.m_DepthTestEnable;
            depth_stencil_state.depthWriteEnable = graphics_blob_ptr->m_DepthStencilState.m_DepthWriteEnable;
            depth_stencil_state.depthCompareOp = graphics_blob_ptr->m_DepthStencilState.m_DepthCompareOp;
            depth_stencil_state.depthBoundsTestEnable = graphics_blob_ptr->m_DepthStencilState.m_DepthBoundsTestEnable;
            depth_stencil_state.stencilTestEnable = graphics_blob_ptr->m_DepthStencilState.m_StencilTestEnable;
            depth_stencil_state.front = graphics_blob_ptr->m_DepthStencilState.m_Front;
            depth_stencil_state.back = graphics_blob_ptr->m_DepthStencilState.m_Back;
            depth_stencil_state.minDepthBounds = graphics_blob_ptr->m_DepthStencilState.m_MinDepthBounds;
            depth_stencil_state.maxDepthBounds = graphics_blob_ptr->m_DepthStencilState.m_MaxDepthBounds;

            Array<VkPipelineColorBlendAttachmentState> blend_attachments(graphics_blob_ptr->m_ColorAttachmentCount);
            for (uint32_t i = 0; i < graphics_blob_ptr->m_ColorAttachmentCount; ++i)
            {
                blend_attachments[i].blendEnable = graphics_blob_ptr->m_BlendAttachments[i].m_BlendEnable;
                blend_attachments[i].srcColorBlendFactor = graphics_blob_ptr->m_BlendAttachments[i].m_SrcColorBlendFactor;
                blend_attachments[i].dstColorBlendFactor = graphics_blob_ptr->m_BlendAttachments[i].m_DstColorBlendFactor;
                blend_attachments[i].colorBlendOp = graphics_blob_ptr->m_BlendAttachments[i].m_ColorBlendOp;
                blend_attachments[i].srcAlphaBlendFactor = graphics_blob_ptr->m_BlendAttachments[i].m_SrcAlphaBlendFactor;
                blend_attachments[i].dstAlphaBlendFactor = graphics_blob_ptr->m_BlendAttachments[i].m_DstAlphaBlendFactor;
                blend_attachments[i].alphaBlendOp = graphics_blob_ptr->m_BlendAttachments[i].m_AlphaBlendOp;
                blend_attachments[i].colorWriteMask = graphics_blob_ptr->m_BlendAttachments[i].m_ColorWriteMask;
            }

            VkPipelineColorBlendStateCreateInfo blend_state = {};
            blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            blend_state.logicOpEnable = VK_FALSE;
            blend_state.attachmentCount = blend_attachments.Count();
            blend_state.pAttachments = blend_attachments.Data();

            VkPipelineMultisampleStateCreateInfo multisample_state = {};
            multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state.sampleShadingEnable = VK_FALSE;
            multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineViewportStateCreateInfo viewport_state = {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = NULL;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = NULL;

            const VkDynamicState dynamic_states[] =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamic_state = {};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(sizeof(dynamic_states) / sizeof(VkDynamicState));
            dynamic_state.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipeline_info = {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.layout = tech->m_PipelineLayout;
            pipeline_info.renderPass = tech->m_RenderPass;
            pipeline_info.stageCount = fs_module != VK_NULL_HANDLE ? 2 : 1;
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_state;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer_state;
            pipeline_info.pMultisampleState = &multisample_state;
            pipeline_info.pDepthStencilState = &depth_stencil_state;
            pipeline_info.pColorBlendState = &blend_state;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
            VK(vkCreateGraphicsPipelines(device->m_Device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &tech->m_Pipeline));

            vkDestroyShaderModule(device->m_Device, vs_module, NULL);
            vkDestroyShaderModule(device->m_Device, fs_module, NULL);
        }
    }
    else
    {
        size_t cs_size;
        const uint32_t* cs_code = static_cast<const uint32_t*>(stream.Read(&cs_size));

        ASSERT(stream.IsEndOfStream());

        // Pipeline
        {
            VkShaderModule cs_module = VK_NULL_HANDLE;

            VkShaderModuleCreateInfo cs_module_create_info = {};
            cs_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            cs_module_create_info.codeSize = cs_size;
            cs_module_create_info.pCode = cs_code;
            VK(vkCreateShaderModule(device->m_Device, &cs_module_create_info, NULL, &cs_module));

            VkPipelineShaderStageCreateInfo shader_stage = {};
            shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shader_stage.module = cs_module;
            shader_stage.pName = "main";

            VkComputePipelineCreateInfo pipeline_info = {};
            pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipeline_info.layout = tech->m_PipelineLayout;
            pipeline_info.stage = shader_stage;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
            VK(vkCreateComputePipelines(device->m_Device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &tech->m_Pipeline));

            vkDestroyShaderModule(device->m_Device, cs_module, NULL);
        }
    }

#ifdef _DEBUG
    if (old_tech)
    {
        for (GfxRenderSetup setup = tech->m_RenderSetupHead; setup != NULL; setup = setup->m_Next)
        {
            vkDestroyFramebuffer(device->m_Device, setup->m_Framebuffer, NULL);
            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = tech->m_RenderPass;
            framebuffer_info.attachmentCount = setup->m_ImageViews.Count();
            framebuffer_info.pAttachments = setup->m_ImageViews.Data();
            framebuffer_info.width = setup->m_Extent.width;
            framebuffer_info.height = setup->m_Extent.height;
            framebuffer_info.layers = 1;
            VK(vkCreateFramebuffer(device->m_Device, &framebuffer_info, NULL, &setup->m_Framebuffer));
        }
    }
    else
    {
        tech->m_RenderSetupHead = NULL;
        tech->m_RenderSetupTail = NULL;
    }
#endif

    return tech;
}

void GfxDestroyTechnique(GfxDevice device, GfxTechnique tech)
{
    if (tech != NULL)
    {
        vkDestroyPipeline(device->m_Device, tech->m_Pipeline, NULL);
        vkDestroyPipelineLayout(device->m_Device, tech->m_PipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device->m_Device, tech->m_DescriptorSetLayout, NULL);
        if (tech->m_BindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
            vkDestroyRenderPass(device->m_Device, tech->m_RenderPass, NULL);
        Delete<GfxTechnique_T>(tech);
    }
}

GfxTechnique GfxLoadTechnique(GfxDevice device, const char* filepath)
{
    const size_t filepath_len = strlen(filepath);

    uint64_t hash = GfxHash(filepath, filepath_len);
    if (GfxDevice_T::TechniqueEntry* tech_entry = device->m_TechniqueEntries.Find(hash))
    {
        return tech_entry->m_Technique;
    }

    size_t blob_filepath_offset = 0;
    while (blob_filepath_offset < filepath_len &&
        (filepath[blob_filepath_offset] == '.' ||
         filepath[blob_filepath_offset] == '/' ||
         filepath[blob_filepath_offset] == '\\'))
    {
        ++blob_filepath_offset;
    }
    String blob_filepath("Data/%s.blob", filepath + blob_filepath_offset);

    void* json_data = NULL;
    void* blob_data = NULL;
    size_t json_size = 0;
    size_t blob_size = 0;
    bool json_loaded = ReadFile(filepath, "r", &json_data, &json_size);
    bool blob_loaded = ReadFile(blob_filepath.Data(), "rb", &blob_data, &blob_size);
    if (!json_loaded && !blob_loaded)
    {
        Print("Error: Failed to read from file %s", filepath);
        return NULL;
    }

    bool create_new_blob = false;
    if (json_loaded && blob_loaded)
    {
        uint64_t json_checksum = GfxHash(static_cast<const char*>(json_data), json_size);
        uint64_t blob_checksum = *static_cast<const uint64_t*>(blob_data);
        create_new_blob = json_checksum != blob_checksum;
    }
    else if (json_loaded)
    {
        create_new_blob = true;
    }

    Blob blob;
    if (create_new_blob)
    {
        blob = CreateTechniqueBlob(json_data, json_size);
        while (!blob.m_Data || !blob.m_Size)
        {
            Print("Failed to parse file %s. Press a key to reload...", filepath);
            getchar();

            Free(json_data);

            json_loaded = ReadFile(filepath, "r", &json_data, &json_size);
            if (!json_loaded)
            {
                Print("Error: Failed to read from file %s", filepath);
                if (blob_loaded)
                    Free(blob_data);
                return NULL;
            }

            blob = CreateTechniqueBlob(json_data, json_size);
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

    GfxDevice_T::TechniqueEntry tech_entry;
    tech_entry.m_Technique = GfxCreateTechnique(device, blob.m_Data, blob.m_Size);
    tech_entry.m_Filepath = filepath;
    tech_entry.m_Checksum = *static_cast<const uint64_t*>(blob.m_Data);
    device->m_TechniqueEntries.Put(hash, tech_entry);

    if (create_new_blob)
        DestroyBlob(blob);
    if (json_loaded)
        Free(json_data);
    if (blob_loaded)
        Free(blob_data);

    Print("Loaded %s", create_new_blob ? filepath : blob_filepath.Data());

    return tech_entry.m_Technique;
}

#ifdef _DEBUG
void GfxReloadAllTechniques(GfxDevice device)
{
    const uint32_t capacity = device->m_TechniqueEntries.Capacity();
    for (uint32_t i = 0; i < capacity; ++i)
    {
        GfxDevice_T::TechniqueEntry* tech_entry = device->m_TechniqueEntries.Get(i);
        if (tech_entry == NULL)
        {
            continue;
        }

        void* json_data = NULL;
        size_t json_size = 0;
        String json_filepath("%s", tech_entry->m_Filepath.Data());
        if (!ReadFile(json_filepath.Data(), "r", &json_data, &json_size))
        {
            continue;
        }

        uint64_t checksum = GfxHash(static_cast<const char*>(json_data), json_size);
        if (checksum == tech_entry->m_Checksum)
        {
            free(json_data);
            continue;
        }

        Blob blob = CreateTechniqueBlob(json_data, json_size);
        if (!blob.m_Data || !blob.m_Size)
        {
            free(json_data);
            continue;
        }

        size_t blob_filepath_offset = 0;
        while (blob_filepath_offset < tech_entry->m_Filepath.Length() &&
            (tech_entry->m_Filepath.Data()[blob_filepath_offset] == '.' ||
             tech_entry->m_Filepath.Data()[blob_filepath_offset] == '/' ||
             tech_entry->m_Filepath.Data()[blob_filepath_offset] == '\\'))
        {
            ++blob_filepath_offset;
        }
        String blob_filepath("Data/%s.blob", tech_entry->m_Filepath.Data() + blob_filepath_offset);
        if (!WriteFile(blob_filepath.Data(), "wb", blob.m_Data, blob.m_Size))
        {
            Print("Error: Failed to write to file %s", blob_filepath.Data());
        }

        tech_entry->m_Technique = GfxCreateTechnique(device, blob.m_Data, blob.m_Size, tech_entry->m_Technique);
        tech_entry->m_Checksum = checksum;

        DestroyBlob(blob);
        free(json_data);

        Print("Loaded %s", json_filepath.Data());
    }
}
#endif
