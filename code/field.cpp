V2
coords_field_to_world(Field *field, V2 pos)
{
    V2 top_left = v2(field->center_world.x - field->dim_world.x/2.0f, field->center_world.y - field->dim_world.y/2.0f);
    pos.x = pos.x / (Float)field->width * field->dim_world.x + top_left.x;
    pos.y = pos.y / (Float)field->height * field->dim_world.y + top_left.y;
    return pos;
}
V2
coords_world_to_field(Field *field, V2 pos)
{
    V2 top_left = v2(field->center_world.x - field->dim_world.x/2.0f, field->center_world.y - field->dim_world.y/2.0f);
    pos.x = (pos.x - top_left.x) / field->dim_world.x * (Float(field->width));
    pos.y = (pos.y - top_left.y) / field->dim_world.y * (Float(field->height));
    return pos;
}
Float
scale_world_to_field(Field *field, Float s)
{
    return s / field->dim_world.x * (Float(field->width));
}
Float
scale_field_to_world(Field *field, Float s)
{
    return s / (Float(field->width)) * field->dim_world.x;
}

// From: https://gamedev.stackexchange.com/questions/152991/how-can-i-calculate-normals-using-a-vertex-and-index-buffer
Void
calculate_vertex_normals(Field *field)
{
    Float *vertices = field->vertices;
    UInt *indices = field->indices;
    // For each face, compute the face normal, and accumulate it into each vertex.
    for(Int index = 0; index < (field->width-1)*6 * (field->height-1); index += 3) {
        Int vertexA = indices[index];
        Int vertexB = indices[index + 1];
        Int vertexC = indices[index + 2];
        
        V3 edgeAB = v3(&(vertices[vertexB*9])) - v3(&(vertices[vertexA*9]));
        V3 edgeAC = v3(&(vertices[vertexC*9])) - v3(&(vertices[vertexA*9]));
        
        // The cross product is perpendicular to both input vectors (normal to the plane).
        // Flip the argument order if you need the opposite winding.    
        V3 areaWeightedNormal = v3(glm::cross(edgeAB.to_glm(), edgeAC.to_glm()));
        
        // Don't normalize this vector just yet. Its magnitude is proportional to the
        // area of the triangle (times 2), so this helps ensure tiny/skinny triangles
        // don't have an outsized impact on the final normal per vertex.
        
        // Accumulate this cross product into each vertex normal slot.
        vertices[vertexA*9 + 6] += areaWeightedNormal.x;
        vertices[vertexA*9 + 7] += areaWeightedNormal.y;
        vertices[vertexA*9 + 8] += areaWeightedNormal.z;
        
        vertices[vertexB*9 + 6] += areaWeightedNormal.x;
        vertices[vertexB*9 + 7] += areaWeightedNormal.y;
        vertices[vertexB*9 + 8] += areaWeightedNormal.z;
        
        vertices[vertexC*9 + 6] += areaWeightedNormal.x;
        vertices[vertexC*9 + 7] += areaWeightedNormal.y;
        vertices[vertexC*9 + 8] += areaWeightedNormal.z;
    }
    
    // Finally, normalize all the sums to get a unit-length, area-weighted average.
    for(int vertex = 0; vertex < (field->width) * (field->height); vertex++)
    {
        V3 current_normal = v3(&vertices[vertex*9 + 6]);
        V3 new_normal = current_normal.normalized();
#if 1
        vertices[vertex*9 + 6] = new_normal.x;
        vertices[vertex*9 + 7] = new_normal.y;
        vertices[vertex*9 + 8] = new_normal.z;
#endif
    }
}

Void
fill_field_render_data(Field *field)
{
    if(!field->render_data_allocated)
    {
        // cpu
        field->vertices = (Float *)alloc(sizeof(Float) * field->width*9 * field->height);
        field->indices = (UInt *)alloc(sizeof(UInt) * (field->width-1)*6 * (field->height-1));
        
        // gpu
        field->ebos = (UInt *)alloc(sizeof(UInt) * (field->height-1));
        
        field->vertex_data.vao = gpu_gen_vao();
        field->vertex_data.vbo = gpu_gen_vbo();
        for(Int i = 0; i < field->height-1; i++)
        {
            field->ebos[i] = gpu_gen_ebo();
        }
        
        field->render_data_allocated = true;
    }
    
    for(Int y = 0; y < field->height; y++)
    {
        for(Int x = 0; x < field->width; x++)
        {
            Int stride = 9;
            
            V2 coords_world = coords_field_to_world(field, v2(x, y));
            field->vertices[y*field->width*stride + x*stride + 0] = coords_world.x;
            field->vertices[y*field->width*stride + x*stride + 1] = field->points[y][x].height;
            field->vertices[y*field->width*stride + x*stride + 2] = coords_world.y;
            
            field->vertices[y*field->width*stride + x*stride + 3] = field->points[y][x].color.r;
            field->vertices[y*field->width*stride + x*stride + 4] = field->points[y][x].color.g;
            field->vertices[y*field->width*stride + x*stride + 5] = field->points[y][x].color.b;
            
        }
    }
    
    for(Int y = 0; y < field->height - 1; y++)
    {
        for(Int x = 0; x < field->width - 1; x++)
        {
            Int stride = 6;
            
            field->indices[y*stride*(field->width-1) + x*stride + 0] = x + y*field->width;
            field->indices[y*stride*(field->width-1) + x*stride + 1] = x + (y+1)*field->width;
            field->indices[y*stride*(field->width-1) + x*stride + 2] = x + 1 + (y+1)*field->width;
            
            field->indices[y*stride*(field->width-1) + x*stride + 3] = x + y*field->width;
            field->indices[y*stride*(field->width-1) + x*stride + 4] = x + 1 + (y+1)*field->width;
            field->indices[y*stride*(field->width-1) + x*stride + 5] = x + 1 + y*field->width;
        }
    }
    
    calculate_vertex_normals(field);
    
    U64 vertex_count = field->width*9 * field->height;
    gpu_upload_vertices_stream(field->vertex_data.vbo, field->vertices, sizeof(Float)*vertex_count);
    
    for(Int i = 0; i < field->height-1; i++)
    {
        gpu_upload_indices(field->ebos[i], &(field->indices[i*(field->width-1)*6]),
                           sizeof(UInt) * (field->width-1)*6);
    }
    
    gpu_set_vao_attribute(field->vertex_data.vao, field->vertex_data.vbo, 0, 3, sizeof(Float)*9, 0);
    gpu_set_vao_attribute(field->vertex_data.vao, field->vertex_data.vbo, 1, 3, sizeof(Float)*9, sizeof(Float)*3);
    gpu_set_vao_attribute(field->vertex_data.vao, field->vertex_data.vbo, 2, 3, sizeof(Float)*9, sizeof(Float)*6);
}

Field
create_field(Int width, Int height)
{
    Field result;
    result.width = width;
    result.height = height;
    
    result.points = (FieldPoint **)alloc(sizeof(FieldPoint *) * height);
    result.target_points = (FieldPoint **)alloc(sizeof(FieldPoint *) * height);
    for(Int y = 0; y < result.height; y++)
    {
        result.points[y] = (FieldPoint *)alloc(sizeof(FieldPoint) * width);
        result.target_points[y] = (FieldPoint *)alloc(sizeof(FieldPoint) * width);
        
        for(Int x = 0; x < result.width; x++)
        {
            result.points[y][x].height = 0;
            result.points[y][x].color = color(0, 0, 0, 1.0f);
        }
    }
    
    return result;
}

Void
field_draw_small_bitmap(Field *field, SmallFieldBitmap bitmap, V2I offset,
                        Float added_height, Color color,
                        Bool set_base_height = false, Float base_height = 1) // offset from center
{
    V2I draw_pos = offset;
    
    for(Int y = 0; y < 5; y++)
    {
        for(Int x = 0; x < 5; x++)
        {
            V2I coords = v2i(draw_pos.x + x, draw_pos.y + y);
            if(coords.x < 0 || coords.x >= field->width ||
               coords.y < 0 || coords.y >= field->height)
                break;
            
            FieldPoint *point = &(field->target_points[coords.y][coords.x]);
            if(bitmap.data[y][x])
            {
                if(set_base_height)
                    point->height = base_height;
                point->height += added_height;
                point->color = color;
            }
        }
    }
}

Void // coords in world space
field_draw_circle(Field *field, V2 center, Float radius, Float added_height, Color color,
                  Bool set_base_height = false, Float base_height = 1)
{
    V2 center_field = coords_world_to_field(field, center);
    Float radius_field = scale_world_to_field(field, radius);
    V2I top_left = v2i(center_field - v2(radius_field, radius_field));
    V2I bottom_right = v2i(center_field + v2(radius_field + 0.5f, radius_field + 0.5f));
    
    for(Int y = top_left.y; y < bottom_right.y; y++)
    {
        for(Int x = top_left.x; x < bottom_right.x; x++)
        {
            V2I coords = v2i(x, y);
            if(coords.x < 0 || coords.x >= field->width ||
               coords.y < 0 || coords.y >= field->height)
                continue;
            
            Float dist = v2_dist(center_field, v2(x, y));
            if(dist <= radius_field)
            {
                FieldPoint *point = &(field->target_points[y][x]);
                if(set_base_height)
                    point->height = base_height;
                point->height += added_height;
                point->color = color;
            }
        }
    }
}

Void
field_draw_playing_area(GameState *game_state, Field *field, Float height, Color area_color)
{
    field->playing_area_dim = v2(10.5, 7);
    V2 raised_area_top_left_field = coords_world_to_field(field, v2(field->playing_area_dim.x / -2, field->playing_area_dim.y / -2)) - v2(1, 1);
    V2 raised_area_bottom_right_field = coords_world_to_field(field, v2(field->playing_area_dim.x / 2, field->playing_area_dim.y / 2)) + v2(1, 1);
    
    for(Int y = raised_area_top_left_field.y; y <= raised_area_bottom_right_field.y; y++)
    {
        for(Int x = raised_area_top_left_field.x; x <= raised_area_bottom_right_field.x; x++)
        {
            FieldPoint *point = &(field->target_points[y][x]);
            point->height = height;
            point->color = area_color;
        }
    }
}

Void
field_draw_text(GameState *game_state, Field *field, const Char *str, V2 pos,
                Float added_height, Color color, Bool set_base_height = false, Float base_height = 1)
{
    V2I start_pos = v2i(coords_world_to_field(field, pos));
    
    for(Int i = 0; str[i] != '\0'; i++)
    {
        UInt index = 0;
        if(str[i] == ' ')
            continue;
        if(str[i] >= 'A' && str[i] <= 'Z')
            index = str[i] - 'A';
        else if(str[i] >= 'a' && str[i] <= 'z')
            index = str[i] - 'a';
        else if(str[i] >= '0' && str[i] <= '9')
            index = 26 + str[i] - '0';
        
        field_draw_small_bitmap(field, game_state->text_bitmaps[index], start_pos + v2i(6*i, 0),
                                added_height, color, set_base_height, base_height);
    }
}


Void
update_field_data_main_menu(GameState *game_state, Field *field)
{
    F64 time = get_time();
    
    for(Int y = 0; y < field->height; y++)
    {
        for(Int x = 0; x < field->width; x++)
        {
            FieldPoint *point = &(field->target_points[y][x]);
            point->height = 0;
            //point->height += random_float(0, 0.1f);
            point->color = color(0.31, 0.58, 0.68, 1);
            
            //point->height += sin((x+0.5984f)*0.2f + time) * 0.1f;
            point->height += sin((x+0.5984f)*0.1f + time) * 0.3f;
            point->height += sin((-y+x)*0.2f + time) * 0.07f;
            point->height += cos((y+x)*0.3f + time) * 0.07f;
        }
    }
    
    {
        Float menu_text_left = -5.0f;
        Float menu_text_spacing = 1.0f;
        Float menu_text_y = -2.0f;
        
        field_draw_text(game_state, field, "Tempest", v2(menu_text_left, menu_text_y), 0.12f, color(0.92, 0.33, 0.53, 1.0f));
        
        Color normal_text_color = color(0.92, 0.69, 0.33, 1.0f);
        Color selected_text_color = color(0.92, 0.85, 0.73, 1.0f);
        selected_text_color = interpolate(selected_text_color, normal_text_color, sin(time*5));
        
        Int selector = game_state->main_menu_selector;
        
        menu_text_y += menu_text_spacing * 2;
        field_draw_text(game_state, field, "Play", v2(menu_text_left, menu_text_y), 0.12f,
                        selector == 0 ? selected_text_color : normal_text_color);
        menu_text_y += menu_text_spacing;
        field_draw_text(game_state, field, "Exit", v2(menu_text_left, menu_text_y), 0.12f,
                        selector == 1 ? selected_text_color : normal_text_color);
    }
    
    {
        Char text_buffer[40];
        Color normal_text_color = color(0.92, 0.69, 0.33, 1.0f);
        
        Float menu_text_left = 1.0f;
        Float menu_text_spacing = 1.0f;
        Float menu_text_y = -2.0f;
        
        // Later: color literals (Color.white, Color.red, etc)
        field_draw_text(game_state, field, "Records", v2(menu_text_left, menu_text_y), 0.12f, normal_text_color);
        
        menu_text_y += menu_text_spacing * 2;
        sprintf(text_buffer, "time %d", (Int)game_state->save.highest_time);
        field_draw_text(game_state, field, text_buffer, v2(menu_text_left, menu_text_y), 0.12f, normal_text_color);
        
        menu_text_y += menu_text_spacing;
        sprintf(text_buffer, "kills %d", (Int)game_state->save.highest_kills);
        field_draw_text(game_state, field, text_buffer, v2(menu_text_left, menu_text_y), 0.12f, normal_text_color);
    }
}

Void
update_field_data_in_game(GameState *game_state, Field *field)
{
    F64 time = get_time();
    Color ocean_color = color(0.31, 0.58, 0.68, 1);
    Color play_area_color = color(0.17, 0.55, 0.42, 1);
    for(Int y = 0; y < field->height; y++)
    {
        for(Int x = 0; x < field->width; x++)
        {
            FieldPoint *point = &(field->target_points[y][x]);
            point->height = 0;
            //point->height += random_float(0, 0.1f);
            point->color = ocean_color;
            
            //point->height += sin((x+0.5984f)*0.2f + time) * 0.1f;
            point->height += sin((x+0.5984f)*0.1f + time) * 0.3f;
            point->height += sin((-y+x)*0.2f + time) * 0.07f;
            point->height += cos((y+x)*0.3f + time) * 0.07f;
        }
    }
    
    Float playing_area_height = 1;
    if(game_state->paused)
    {
        field_draw_playing_area(game_state, field, playing_area_height, play_area_color);
        //field_draw_text(game_state, field, "paused", v2(-1.8f, -1), 0.12f, color(0.96, 0.78, 0.02, 1.0f));
        //field_draw_text(game_state, field, "esc to resume", v2(-3.9f, 1), 0.12f, color(0.96, 0.78, 0.02, 1.0f));
        
        Float menu_text_left = -3.0f;
        Float menu_text_spacing = 1.0f;
        Float menu_text_y = -2.0f;
        
        Color normal_text_color = color(0.92, 0.69, 0.33, 1.0f);
        Color selected_text_color = color(0.92, 0.85, 0.73, 1.0f);
        selected_text_color = interpolate(selected_text_color, normal_text_color, sin(time*5));
        
        Int selector = game_state->pause_menu_selector;
        
        field_draw_text(game_state, field, "Paused", v2(menu_text_left, menu_text_y), 0.12f, color(0.92, 0.33, 0.53, 1.0f));
        menu_text_y += menu_text_spacing * 2;
        
        field_draw_text(game_state, field, "Resume", v2(menu_text_left, menu_text_y), 0.12f,
                        selector == 0 ? selected_text_color : normal_text_color);
        menu_text_y += menu_text_spacing;
        
        field_draw_text(game_state, field, "Main Menu", v2(menu_text_left, menu_text_y), 0.12f,
                        selector == 1 ? selected_text_color : normal_text_color);
        
    }
    else
    {
        Player *player = &game_state->player;
        
        field_draw_playing_area(game_state, field, playing_area_height, play_area_color);
        
        for(Int i = 0; i < game_state->enemy_bullets.length; i++)
        {
            Bullet bullet = game_state->enemy_bullets[i];
            field_draw_circle(field, bullet.pos, bullet.radius, 0.3f, bullet.color);
        }
        for(Int i = 0; i < game_state->enemies.length; i++)
        {
            Enemy enemy = game_state->enemies[i];
            field_draw_circle(field, enemy.pos, enemy.radius, 0.3f, color(1, 0, 0, 0), true, playing_area_height);
            
            Float max_enemy_charge_height = 1.5f;
            Float enemy_charge_height = (enemy.time_between_fires - enemy.time_to_fire) * max_enemy_charge_height;
            // TODO: fix this
            if(enemy.type != EnemyType::suicide)
                field_draw_circle(field, enemy.pos, enemy.radius/2, enemy_charge_height, color(1, 0, 0, 0));
        }
        for(Int i = 0; i < game_state->player_bullets.length; i++)
        {
            Bullet bullet = game_state->player_bullets[i];
            field_draw_circle(field, bullet.pos, bullet.radius, 0.3f, bullet.color);
        }
        for(Int i = 0; i < game_state->enemy_explosions.length; i++)
        {
            EnemyExplosion explosion = game_state->enemy_explosions[i];
            
            Float fraction = explosion.time_left / explosion.time_left_max;
            Float radius = explosion.initial_radius * (1-fraction) * 3;
            Float height = explosion.initial_height * fraction + 0.3f;
            
            field_draw_circle(field, explosion.pos, radius,
                              height, explosion.initial_color);
            field_draw_circle(field, explosion.pos, radius - 0.2f,
                              -height, play_area_color);
        }
        
        if(game_state->life_lost_explosion_enabled)
        {
            Float height = 0.5f;
            
            field_draw_circle(field, game_state->life_lost_explosion_center, game_state->life_lost_explosion_radius,
                              height, color(1, 0.7f, 1, 1));
            field_draw_circle(field, game_state->life_lost_explosion_center, game_state->life_lost_explosion_radius - 0.5f,
                              -height, play_area_color);
            
            if(!game_state->in_tutorial && !game_state->player_dead)
            {
                Char life_warning_text_buffer[40];
                if(game_state->player.lives == 0)
                    sprintf(life_warning_text_buffer, "last life");
                else
                    sprintf(life_warning_text_buffer, "%d lives left", (Int)game_state->player.lives + 1);
                field_draw_text(game_state, field, life_warning_text_buffer, v2(-4.0f, 0), 0.12f, color(1, 1, 1, 1.0f));
            }
        }
        
        
        if(game_state->player_dead)
        {
            Float left = -4.0f;
            Float top = -1.5f;
            Float x = left;
            Float y = top;
            
            Color normal_color = color(1, 1, 1, 1);
            Color highlight_color = color(0.92, 0.33, 0.53, 1.0f);
            
            field_draw_text(game_state, field, "You died", v2(x, y), 0.12f, normal_color);
            
            x = left;
            y += scale_field_to_world(field, 14);
            field_draw_text(game_state, field, "Press ", v2(x, y), 0.12f, normal_color);
            x += scale_field_to_world(field, 6*6);
            field_draw_text(game_state, field, "Enter ", v2(x, y), 0.12f, highlight_color);
            x = left;
            y += scale_field_to_world(field, 8);
            field_draw_text(game_state, field, "to return to", v2(x, y), 0.12f, normal_color);
            x = left;
            y += scale_field_to_world(field, 8);
            field_draw_text(game_state, field, "the main menu", v2(x, y), 0.12f, normal_color);
        }
        
        if(!game_state->player_dead)
        {
            field_draw_circle(field, player->pos, 0.1f, 0.8f, color(1, 1, 1, 1));
            for(Int a = 0; a < player->lives; a++)
            {
                Float angle = ((Float)a / (Float)3) * pi*2;
                V2 dir = v2(cos(angle), sin(angle));
                field_draw_circle(field, player->pos + dir*0.3f, 0.1f, 0.2f, color(1, 1, 1, 1));
            }
        }
    }
    
#if 1
    {
        Char text_buffer[40];
        sprintf(text_buffer, "%d", (Int)game_state->time_in_game);
        Float x_offset = 0;
        if(strlen(text_buffer) == 1)
            x_offset = 12;
        else if(strlen(text_buffer) == 2)
            x_offset = 6;
        x_offset = scale_field_to_world(field, x_offset);
        field_draw_text(game_state, field, "time", v2(-8.5f, -4.0f), 0.12f, color(1, 1, 1, 1.0f));
        field_draw_text(game_state, field, text_buffer, v2(-8.0f + x_offset, -3), 0.12f, color(1, 1, 1, 1.0f));
    }
    {
        Char text_buffer[40];
        sprintf(text_buffer, "%d", (Int)game_state->kills);
        Float x_offset = 0;
        if(strlen(text_buffer) == 1)
            x_offset = 12;
        else if(strlen(text_buffer) == 2)
            x_offset = 6;
        x_offset = scale_field_to_world(field, x_offset);
        field_draw_text(game_state, field, "kill", v2(6.0f, -4.0f), 0.12f, color(1, 1, 1, 1.0f));
        field_draw_text(game_state, field, text_buffer, v2(6.0f + x_offset, -3), 0.12f, color(1, 1, 1, 1.0f));
    }
#endif
    
    if(game_state->in_tutorial)
    {
        Float left = -5.0f;
        Color normal_color = color(1, 1, 1, 1);
        Color highlight_color = color(0.92, 0.33, 0.53, 1.0f);
        
        Float continue_text_y = 2.0f;
        Float x = left;
        
        field_draw_text(game_state, field, "Press ", v2(x, continue_text_y), 0.12f, normal_color);
        x += scale_field_to_world(field, 6*6);
        field_draw_text(game_state, field, "Enter ", v2(x, continue_text_y), 0.12f, highlight_color);
        
        if(game_state->tutorial_phase == 3)
        {
            x = left;
            field_draw_text(game_state, field, "to ", v2(x, continue_text_y + scale_field_to_world(field, 6)), 0.12f, normal_color);
            x += scale_field_to_world(field, 3*6);
            field_draw_text(game_state, field, "start game", v2(x, continue_text_y + scale_field_to_world(field, 6)), 0.12f, highlight_color);
        }
        else
        {
            field_draw_text(game_state, field, "to continue", v2(left, continue_text_y + scale_field_to_world(field, 6)), 0.12f, normal_color);
        }
        
        Float instruction_text_y = -3.0f;
        if(game_state->tutorial_phase == 0)
        {
            x = left;
            field_draw_text(game_state, field, "wasd", v2(x, instruction_text_y), 0.12f, highlight_color);
            x += scale_field_to_world(field, 5*6);
            field_draw_text(game_state, field, "to move", v2(x, instruction_text_y), 0.12f, normal_color);
            
            instruction_text_y += scale_field_to_world(field, 8);
            x = left;
            field_draw_text(game_state, field, "arrows ", v2(x, instruction_text_y), 0.12f, highlight_color);
            x += scale_field_to_world(field, 7*6);
            field_draw_text(game_state, field, "to shoot", v2(x, instruction_text_y), 0.12f, normal_color);
            
        }
        else if(game_state->tutorial_phase == 1)
        {
            x = left;
            field_draw_text(game_state, field, "enemies ", v2(x, instruction_text_y), 0.12f, highlight_color);
            x += scale_field_to_world(field, 8*6 - 3);
            field_draw_text(game_state, field, "are ", v2(x, instruction_text_y), 0.12f, normal_color);
            x += scale_field_to_world(field, 4*6 - 3);
            field_draw_text(game_state, field, "red", v2(x, instruction_text_y), 0.12f, color(1, 0, 0, 1));
            
            instruction_text_y += scale_field_to_world(field, 8);
            x = left;
            field_draw_text(game_state, field, "bullets ", v2(x, instruction_text_y), 0.12f, highlight_color);
            x += scale_field_to_world(field, 8*6 - 3);
            field_draw_text(game_state, field, "are", v2(x, instruction_text_y), 0.12f, normal_color);
            x += scale_field_to_world(field, 4*6 - 3);
            field_draw_text(game_state, field, "yellow", v2(x, instruction_text_y), 0.12f, color(1, 1, 0, 1));
            
            
            
        }
        else if(game_state->tutorial_phase == 2)
        {
            x = left;
            field_draw_text(game_state, field, "kill them", v2(x, instruction_text_y), 0.12f, normal_color);
        }
        else if(game_state->tutorial_phase == 3)
        {
            x = left;
            // TODO: add exclamation point!
            field_draw_text(game_state, field, "good job", v2(x, instruction_text_y), 0.12f, normal_color);
            
            instruction_text_y += scale_field_to_world(field, 8);
            x = left;
            field_draw_text(game_state, field, "end of tutorial", v2(x, instruction_text_y), 0.12f, normal_color);
            
            instruction_text_y += scale_field_to_world(field, 13);
            x = left;
            field_draw_text(game_state, field, "survive as long", v2(x, instruction_text_y), 0.12f, normal_color);
            instruction_text_y += scale_field_to_world(field, 8);
            x = left;
            field_draw_text(game_state, field, "and kill as many", v2(x, instruction_text_y), 0.12f, normal_color);
            instruction_text_y += scale_field_to_world(field, 8);
            x = left;
            field_draw_text(game_state, field, "as you can", v2(x, instruction_text_y), 0.12f, normal_color);
        }
    }
}

Void
update_field_data(GameState *game_state, Field *field)
{
    if(game_state->in_game)
    {
        update_field_data_in_game(game_state, field);
    }
    else
    {
        update_field_data_main_menu(game_state, field);
    }
    
    for(Int y = 0; y < field->height; y++)
    {
        for(Int x = 0; x < field->width; x++)
        {
            FieldPoint *target_point = &(field->target_points[y][x]);
            FieldPoint *point = &(field->points[y][x]);
            Float up_interp_speed = 60.0f * game_state->d_time;
            Float down_interp_speed = 60.0f * game_state->d_time;
            
            //point->height = target_point->height;
            //point->color = target_point->color;
            if(point->height < target_point->height)
            {
                point->height = interpolate(point->height, target_point->height, up_interp_speed);
                point->color = interpolate(point->color, target_point->color, up_interp_speed);
            }
            else
            {
                point->height = interpolate(point->height, target_point->height, down_interp_speed);
                point->color = interpolate(point->color, target_point->color, down_interp_speed);
            }
            
        }
    }
}