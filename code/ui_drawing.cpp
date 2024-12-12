Void
setup_rect_mesh(Mesh *mesh)
{
    float vertices[] = {
        1, 1, 0, // top right
        1, -1, 0, // bottom right
        -1, -1, 0, // bottom left
        -1, 1, 0, // top left
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    
    UInt vao = gpu_gen_vao();
    UInt vbo = gpu_gen_vbo();
    UInt ebo = gpu_gen_ebo();
    
    gpu_upload_vertices(vbo, vertices, sizeof(vertices));
    gpu_upload_indices(ebo, indices, sizeof(indices));
    
    gpu_vao_attach_ebo(vao, ebo);
    gpu_vao_attach_vbo_attribute(vao, vbo, 0, 3, sizeof(Float)*3, 0);
    
    mesh->vao = vao;
    mesh->vbo = vbo;
    mesh->ebo = ebo;
    mesh->has_ebo = true;
}

V2 dim_ui_to_opengl(V2 window_dim, V2 dim)
{
    V2 result = dim;
    
    result.x *= 1.0f / window_dim.x;
    result.y *= 1.0f / window_dim.y;
    
    return result;
}
V2 pos_ui_to_opengl(V2 window_dim, V2 pos)
{
    V2 result = pos;
    
    result.y *= -1;
    
    result.x -= window_dim.x / 2.0f;
    result.y += window_dim.y / 2.0f;
    
    result.x *= 2.0f / window_dim.x;
    result.y *= 2.0f / window_dim.y;
    
    return result;
}

Void
ui_draw_rect(GameState *game_state, Float x, Float y, Float w, Float h, Color fg_color)
{
    Mesh *mesh = &game_state->rect_mesh;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    gpu_use_shader_program(&game_state->shape_sp);
    
    V2 scale = dim_ui_to_opengl(game_state->window_dim, v2(w, h));
    V2 offset = pos_ui_to_opengl(game_state->window_dim, v2(x+w/2, y+h/2));
    
    gpu_set_uniform_2f(&game_state->shape_sp, "shape_scale", scale.components);
    gpu_set_uniform_2f(&game_state->shape_sp, "shape_offset", offset.components);
    
    gpu_set_uniform_4f(&game_state->shape_sp, "shape_color", fg_color.components);
    
    gpu_draw_indices(mesh->vao, GL_TRIANGLES, 6);
}

Void
ui_draw_rect(GameState *game_state, Rect r, Color fg_color)
{
    ui_draw_rect(game_state, r.x, r.y, r.w, r.h, fg_color);
}






Void
load_debug_font(GameState *game_state)
{
    Int width, height, channel_count;
    stbi_set_flip_vertically_on_load(true);
    U8 *font_image_data = stbi_load(GAME_DATA_DIRECTORY "/textures/debug_font.png",
                                    &width, &height, &channel_count, 0);
    ASSERT(font_image_data);
    
    UInt texture_id = gpu_gen_texture();
    
    gpu_set_texture_wrapping_repeat(texture_id);
    gpu_set_texture_filter_nearest(texture_id);
    
    gpu_upload_image(texture_id, font_image_data, width, height);
    stbi_image_free(font_image_data);
    
    // TODO: send texture coords as a uniform
    Float tex_coord_width = 1.0f;
    float font_vertices[] = {
        // positions        // texture coords
        7, 0, 0.0f,      tex_coord_width, -tex_coord_width,   // top right
        7, 8, 0.0f,       tex_coord_width, 0.0f,   // bottom right
        0, 8, 0.0f,       0.0f,            0.0f,   // bottom left
        0, 0, 0.0f,       0.0f,            -tex_coord_width,    // top left 
    };
    
    unsigned int font_indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    UInt vao = gpu_gen_vao();
    UInt vbo = gpu_gen_vbo();
    UInt ebo = gpu_gen_ebo();
    
    gpu_upload_vertices(vbo, font_vertices, sizeof(font_vertices));
    gpu_upload_indices(ebo, font_indices, sizeof(font_indices));
    
    gpu_vao_attach_ebo(vao, ebo);
    gpu_vao_attach_vbo_attribute(vao, vbo, 0, 3, sizeof(Float)*5, 0);
    gpu_vao_attach_vbo_attribute(vao, vbo, 1, 2, sizeof(Float)*5, sizeof(Float)*3);
    
    game_state->font_texture = texture_id;
    game_state->font_vao = vao;
    game_state->font_vbo = vbo;
    game_state->font_ebo = ebo;
}

Void
ui_draw_debug_text(GameState *game_state, V2 pos, Float scale, const Char *text, Int length, Color fg_color, Color bg_color)
{
    ui_draw_rect(game_state, pos.x, pos.y, length*7*scale, 8*scale, bg_color);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gpu_use_shader_program(&game_state->debug_font_sp);
    
    gpu_set_texture_unit(1, game_state->font_texture);
    gpu_set_uniform_i(&game_state->debug_font_sp, "texture1", 1);
    
    gpu_set_uniform_2f(&game_state->debug_font_sp, "windowSize", game_state->window_dim.components);
    gpu_set_uniform_1f(&game_state->debug_font_sp, "scale", scale);
    
    Float glyph_dim[2] = {7, 8};
    gpu_set_uniform_2f(&game_state->debug_font_sp, "texScale", glyph_dim);
    
    gpu_set_uniform_4f(&game_state->debug_font_sp, "textColor", fg_color.components);
    
    for(Int i = 0; i < length; i++)
    {
        Char c = text[i];
        if(c > 215) c = 0;
        
        Float glyph_offset[2] = {float(7 * (c%16)), float(8 * (c/16))};
        gpu_set_uniform_2f(&game_state->debug_font_sp, "texOffset", glyph_offset);
        
        V2 offset = pos + v2(i*7*scale, 0);
        gpu_set_uniform_2f(&game_state->debug_font_sp, "offset", offset.components);
        
        gpu_draw_indices(game_state->font_vao, GL_TRIANGLES, 6);
    }
}

Void
ui_draw_debug_text(GameState *game_state, V2 pos, Float scale, const Char *text, Color fg_color = {0, 0, 0, 1})
{
    Int length = strlen(text);
    ui_draw_debug_text(game_state, pos, scale, text, length, fg_color, color(0, 0, 0, 0));
}
Void
ui_draw_debug_text(GameState *game_state, V2 pos, Float scale, String string, Color fg_color = {0, 0, 0, 1})
{
    ui_draw_debug_text(game_state, pos, scale, string.data, string.length, fg_color, color(0, 0, 0, 0));
}

Void
ui_draw_log(GameState *game_state, Log *log, Float scale)
{
    Int entries_drawn = 0;
    for(Int i = log->entries.length - 1; entries_drawn < 30 && i >= 0; i--)
    {
        // TODO: make a macro for debug font width and height
        V2 pos = v2(0, game_state->window_dim.y - (scale*8 * (entries_drawn+1)));
        //draw_debug_text(game_state, pos, scale, log->entry_at(i).string.data);
        ASSERT(log->entry_at(i).type != LogEntryType::nonexistant);
        
        String str = log->entry_at(i).string;
        Rect bg_rect = rect(pos.x, pos.y, str.length*7*scale, 8*scale);
        
        //ui_draw_rect(game_state, bg_rect, color(1, 1, 1, 0.3));
        ui_draw_debug_text(game_state, pos, scale, str.data, str.length, Color::white(), color(0, 0, 0, 0.4f));
        
        entries_drawn++;
    }
}

Void
ui_draw_timing_pair(GameState *game_state, V2 pos, Float text_scale,
                    const Char *name, F64 time, Color time_draw_color)
{
    Char profile_text_buffer[200];
    Int extra_spaces = 25 - strlen(name);
    Int buffer_length = 0;
    buffer_length += sprintf(profile_text_buffer+buffer_length, "%s: ", name);
    for(Int i = 0; i < extra_spaces; i++)
    {
        profile_text_buffer[buffer_length] = ' ';
        buffer_length++;
    }
    buffer_length += sprintf(profile_text_buffer+buffer_length, "%.1lf ms", time*1000);
    
    ui_draw_debug_text(game_state, pos, text_scale, profile_text_buffer, buffer_length,
                       time_draw_color, color(0, 0, 0, 0.4f));
}

Void
ui_draw_debug_interface(GameState *game_state, GameMemory *game_memory, Float text_scale)
{
    Char profile_text_buffer[100];
    V2 draw_pos = v2(0, 0);
    Float vertical_spacing = 16;
    Int buffer_length = 0;
    
    String fps_string = create_string("%d fps", (Int)game_state->fps);
    ui_draw_debug_text(game_state, v2(0, 0), text_scale, fps_string.data, fps_string.length, Color::white(), color(0, 0, 0, 0.4f));
    free_string(&fps_string);
    draw_pos.y += 20;
    
    FrameProfile *profile = &(game_state->frame_profiles.last());
    
    for(Int i = 0; i < profile->finished_sections.length; i++)
    {
        Float average = 0;
        for(Int a = 0; a < game_state->frame_profiles.length; a++)
        {
            average += game_state->frame_profiles[a].finished_sections[i].elapsed_time;
        }
        average /= game_state->frame_profiles.length;
        
        SectionProfile *section = &(profile->finished_sections[i]);
        
        ui_draw_timing_pair(game_state, draw_pos, text_scale, section->name, average, Color::white());
        draw_pos.y += vertical_spacing;
    }
    
    {
        Float average = 0;
        for(Int a = 0; a < game_state->frame_profiles.length; a++)
        {
            average += game_state->frame_profiles[a].elapsed_time;
        }
        average /= game_state->frame_profiles.length;
        
        Color ft_draw_color = (profile->elapsed_time < profile->target_max_time) ? Color::white() : color(1, 0.5, 0.5, 1);
        ui_draw_timing_pair(game_state, draw_pos, text_scale, "Frame Time", average, ft_draw_color);
        draw_pos.y += vertical_spacing;
        
        ui_draw_timing_pair(game_state, draw_pos, text_scale, "Max Frame Time", profile->target_max_time, Color::white());
        draw_pos.y += vertical_spacing;
        
        average = 0;
        for(Int a = 0; a < game_state->frame_profiles.length; a++)
        {
            average += game_state->frame_profiles[a].d_time;
        }
        average /= game_state->frame_profiles.length;
        
        ui_draw_timing_pair(game_state, draw_pos, text_scale, "Delta Time", average, Color::white());
        draw_pos.y += vertical_spacing;
    }
    
    
    {
        Float profile_width = 3;
        Float horizontal_spacing = 1;
        Float max_height = 60;
        Float extra_height = 60;
        
        Float top = draw_pos.y + 20;
        Float bottom = top + max_height + extra_height;
        
        Int max_len = sizeof(game_state->frame_profiles.data) / sizeof(FrameProfile);
        ui_draw_rect(game_state, 0, top, max_len*(profile_width+horizontal_spacing), max_height+extra_height, color(0, 0, 0, 0.4f));
        
        ui_draw_debug_text(game_state, v2(5, top+10), text_scale, "Frame Profiles", Color::white());
        
        Color section_colors[4] = {
            Color::green(),
            Color::orange(),
            Color::blue(),
            Color::yellow(),
        };
        
        Float name_draw_height = bottom - text_scale*10;
        for(Int i = 0; i < game_state->frame_profiles[0].finished_sections.length; i++)
        {
            ui_draw_debug_text(game_state, v2(max_len*(horizontal_spacing+profile_width), name_draw_height),
                               text_scale, game_state->frame_profiles[0].finished_sections[i].name, section_colors[i]);
            name_draw_height -= text_scale*10;
        }
        
        for(Int i = 0; i < game_state->frame_profiles.length; i++)
        {
            FrameProfile *profile = &(game_state->frame_profiles.data[i]);
            Float fraction = profile->elapsed_time / profile->target_max_time;
            
            Float profile_height = fraction * max_height;
            Float profile_top = bottom - profile_height;
            Float shade = 0.8f;
            ui_draw_rect(game_state, i*(profile_width+horizontal_spacing), profile_top, profile_width, profile_height, color(shade, shade, shade, 1.0f));
            
            Float last_section_end = 0;
            for(Int a = 0; a < profile->finished_sections.length; a++)
            {
                fraction = profile->finished_sections[a].elapsed_time / profile->elapsed_time;
                Float section_height = profile_height * fraction;
                ui_draw_rect(game_state, i*(profile_width+horizontal_spacing), bottom - last_section_end - section_height,
                             profile_width, section_height, section_colors[a]);
                last_section_end += section_height;
            }
            ui_draw_rect(game_state, game_state->frame_profiles.start*(profile_width+horizontal_spacing) - 1, top, 2, max_height + extra_height, Color::red());
            ui_draw_rect(game_state, 0, bottom - max_height, game_state->frame_profiles.length*(profile_width+horizontal_spacing), 2, Color::blue());
        }
        draw_pos.y += max_height + extra_height;
    }
    
    {
        Float profile_width = 3;
        Float horizontal_spacing = 1;
        Float max_height = 60;
        Float extra_height = 60;
        
        Float top = draw_pos.y + 30;
        Float bottom = top + max_height + extra_height;
        
        Int max_len = sizeof(game_state->frame_profiles.data) / sizeof(FrameProfile);
        ui_draw_rect(game_state, 0, top, max_len*(profile_width+horizontal_spacing), max_height+extra_height, color(0, 0, 0, 0.4f));
        
        ui_draw_debug_text(game_state, v2(5, top+10), text_scale, "Delta Times", Color::white());
        
        for(Int i = 0; i < game_state->frame_profiles.length; i++)
        {
            FrameProfile *profile = &(game_state->frame_profiles.data[i]);
            //Float fraction = profile->elapsed_time / profile->target_max_time;
            Float fraction = profile->d_time / 0.01f;
            
            Float profile_height = fraction * max_height;
            Float profile_top = bottom - profile_height;
            Float shade = 0.8f;
            ui_draw_rect(game_state, i*(profile_width+horizontal_spacing), profile_top, profile_width, profile_height, color(shade, shade, shade, 1.0f));
        }
        ui_draw_rect(game_state, game_state->frame_profiles.start*(profile_width+horizontal_spacing) - 1, top, 2, max_height + extra_height, Color::red());
        ui_draw_rect(game_state, 0, bottom - max_height, game_state->frame_profiles.length*(profile_width+horizontal_spacing), 2, Color::blue());
        draw_pos.y += max_height + extra_height;
        draw_pos.y += 40;
    }
    
    
    // Draw audio buffer
    {
        Float sample_width = 1;
        Float horizontal_spacing = 0;
        Float height = 120;
        Float width = 500;
        
        Float top = draw_pos.y;
        Float bottom = top + height;
        
        Float center = top + height/2;
        
        AudioBuffer *buffer = &game_memory->audio_buffer;
        Int sample_count = sizeof(buffer->data)/2;
        I16 *samples = (I16 *)buffer->data;
        
        ui_draw_rect(game_state, 0, top, sample_count*(sample_width+horizontal_spacing), height, color(0, 0, 0, 0.4f));
        
        ui_draw_debug_text(game_state, v2(5, top+10), text_scale, "Audio Buffer", Color::white());
        
        //Int max = 1000;
        for(Int i = 0; i < width; i++)
        {
            //Float fraction = profile->elapsed_time / profile->target_max_time;
            // NOTE: convert two U8s to U16
            Int sample_index = ((Float)i / (Float)width) * sample_count;
            Float fraction = ((Float)samples[sample_index]) / (Float)32767;
            
            if(fraction < 0)
            {
                Int a = 2;
            }
            
            Float sample_height = fraction * height;
            Float sample_top = bottom - sample_height;
            Float shade = 0.8f;
            //ui_draw_rect(game_state, i*(sample_width+horizontal_spacing), sample_top, sample_width, sample_height, color(shade, shade, shade, 1.0f));
            ui_draw_rect(game_state, i*(sample_width+horizontal_spacing), center + fraction*(height/2), 2, 2, color(shade, shade, shade, 1.0f));
        }
    }
}