V2
coords_field_to_world(Field *field, V2 pos)
{
    // LATER: optimize if possible
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
        
        V3 edgeAB = v3_from_floats(&(vertices[vertexB*9])) - v3_from_floats(&(vertices[vertexA*9]));
        V3 edgeAC = v3_from_floats(&(vertices[vertexC*9])) - v3_from_floats(&(vertices[vertexA*9]));
        
        // The cross product is perpendicular to both input vectors (normal to the plane).
        // Flip the argument order if you need the opposite winding.    
        glm::vec3 _areaWeightedNormal = glm::cross(edgeAB.to_glm(), edgeAC.to_glm());
        V3 areaWeightedNormal = v3_from_glm(_areaWeightedNormal);
        
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
        V3 current_normal = v3_from_floats(&vertices[vertex*9 + 6]);
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
        
        glGenVertexArrays(1, &field->vao);
        glGenBuffers(1, &field->vbo);
        for(Int i = 0; i < field->height-1; i++)
        {
            glGenBuffers(1, &(field->ebos[i]));
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
            
#if 0
            field->vertices[y*field->width*stride + x*stride + 6] = 0;
            field->vertices[y*field->width*stride + x*stride + 7] = 1;
            field->vertices[y*field->width*stride + x*stride + 8] = 0;
#endif
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
    
    glBindVertexArray(field->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, field->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Float) * field->width*9 * field->height, field->vertices, GL_DYNAMIC_DRAW);
    
    for(Int i = 0; i < field->height-1; i++)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field->ebos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UInt) * (field->width-1)*6,
                     &(field->indices[i*(field->width-1)*6]), GL_STATIC_DRAW);
    }
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*9, (Void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*9, (Void *)(sizeof(Float)*3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*9, (Void *)(sizeof(Float)*6));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
                        Float added_height, Color color, Bool set_base_height = false, Float base_height = 1) // offset from center
{
    //V2I draw_pos = v2i(field->width/2 + offset.x, field->height/2 + offset.y);
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
                break;
            
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
field_draw_playing_area(GameState *game_state, Field *field, Float height)
{
    field->playing_area_dim = v2(10.5, 7);
    V2 raised_area_top_left_field = coords_world_to_field(field, v2(field->playing_area_dim.x / -2, field->playing_area_dim.y / -2));
    V2 raised_area_bottom_right_field = coords_world_to_field(field, v2(field->playing_area_dim.x / 2, field->playing_area_dim.y / 2));
    
    for(Int y = raised_area_top_left_field.y; y <= raised_area_bottom_right_field.y; y++)
    {
        for(Int x = raised_area_top_left_field.x; x <= raised_area_bottom_right_field.x; x++)
        {
            FieldPoint *point = &(field->target_points[y][x]);
            point->height = height;
            point->color = color(0.17, 0.55, 0.42, 1);
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
        if(str[i] >= 'a' && str[i] <= 'z')
            index = str[i] - 'a';
        
        field_draw_small_bitmap(field, game_state->text_bitmaps[index], start_pos + v2i(6*i, 0),
                                added_height, color, set_base_height, base_height);
    }
}

Void
update_field_data(GameState *game_state, Field *field)
{
    Float playing_area_height = 1;
    if(game_state->paused)
    {
        field_draw_playing_area(game_state, field, playing_area_height);
        field_draw_text(game_state, field, "paused", v2(-1.8f, -1), 0.12f, color(0.96, 0.78, 0.02, 1.0f));
        field_draw_text(game_state, field, "esc to resume", v2(-3.9f, 1), 0.12f, color(0.96, 0.78, 0.02, 1.0f));
    }
    else
    {
        Player *player = &game_state->player;
        
        //srand(0);
        for(Int y = 0; y < field->height; y++)
        {
            for(Int x = 0; x < field->width; x++)
            {
                FieldPoint *point = &(field->target_points[y][x]);
                point->height = 0;
                point->height += random_float(0, 0.1f);
                point->color = color(0.20, 0.22, 0.30, 1);
            }
        }
        
        field_draw_playing_area(game_state, field, playing_area_height);
        
        field_draw_text(game_state, field, "hiiii", v2(0, 0), 0.12f, color(0.96, 0.78, 0.02, 1.0f));
        
        for(Int i = 0; i < game_state->enemy_bullets.length; i++)
        {
            Bullet bullet = game_state->enemy_bullets[i];
            field_draw_circle(field, bullet.pos, bullet.radius, 0.3f, bullet.color);
        }
        for(Int i = 0; i < game_state->enemies.length; i++)
        {
            Enemy enemy = game_state->enemies[i];
            field_draw_circle(field, enemy.pos, enemy.radius, 0.3f, enemy.color, true, playing_area_height);
            
            Float max_enemy_charge_height = 1.5f;
            Float enemy_charge_height = (enemy.time_between_fires - enemy.time_to_fire) * max_enemy_charge_height;
            field_draw_circle(field, enemy.pos, enemy.radius/2, enemy_charge_height, enemy.color);
        }
        
        field_draw_circle(field, player->pos, 0.1f, 0.8f, color(1, 1, 1, 1));
    }
    
    for(Int y = 0; y < field->height; y++)
    {
        for(Int x = 0; x < field->width; x++)
        {
            FieldPoint *target_point = &(field->target_points[y][x]);
            FieldPoint *point = &(field->points[y][x]);
            Float interp_speed = 60.0f * game_state->d_time;
            
            point->height = interpolate(point->height, target_point->height, interp_speed);
            point->color.interpolate_to(target_point->color, interp_speed);
        }
    }
    
}