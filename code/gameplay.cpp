
Enemy
create_enemy(V2 pos, EnemyType type)
{
    Enemy result;
    result.pos = pos;
    result.radius = 0.3f;
    switch(type)
    {
        case EnemyType::spread:
        {
            result.bullet_speed = 1.0f;
            result.time_between_fires = random_float(0.5f, 2.0f);
            result.amount_per_spread = random_int(8, 20);
        }; break;
        case EnemyType::stream:
        {
            result.bullet_speed = 3.0f;
            result.time_between_fires = random_float(0.5f, 1.0f);
        }; break;
        case EnemyType::spin:
        {
            result.spin_speed = random_float(0.05f, 0.1f);
            result.spin_arm_count = random_int(2, 5);
            result.bullet_speed = 1.0f;
            result.time_between_fires = random_float(0.3f, 0.6f);
        }; break;
        case EnemyType::wall:
        {
            result.wall_dir = v2(random_float(-1, 1), random_float(-1, 1));
            if(result.wall_dir.x == 0 && result.wall_dir.y == 0)
                result.wall_dir.x = 1;
            result.wall_dir = result.wall_dir.normalized();
            result.bullet_speed = 1.0f;
            result.time_between_fires = random_float(0.1f, 0.2f);
        }; break;
        case EnemyType::bomb:
        {
            result.bullet_speed = 1.0f;
            result.time_between_fires = random_float(2.0f, 5.0f);
        }; break;
        case EnemyType::suicide:
        {
            result.suicide_move_speed = 2.0f;
        }; break;
    }
    
    result.time_to_fire = result.time_between_fires;
    result.type = type;
    return result;
}


Void
player_subtract_life(GameState *game_state)
{
    if(!game_state->in_tutorial)
        game_state->player.lives--;
    
    game_state->life_lost_explosion_enabled = true;
    game_state->life_lost_explosion_radius = 0;
    game_state->life_lost_explosion_center = game_state->player.pos;
    
    if(game_state->player.lives < 0)
    {
        game_state->player_dead = true;
    }
}

Void
start_game(GameState *game_state)
{
    game_state->in_game = true;
    game_state->player.pos = v2(0, 0);
    game_state->player.lives = 3;
    game_state->player_bullets.length = 0;
    game_state->enemy_bullets.length = 0;
    game_state->enemies.length = 0;
    game_state->life_lost_explosion_enabled = false;
    game_state->time_in_game = 0;
    game_state->kills = 0;
    game_state->player.powerup = PowerupType::none;
    game_state->last_spawn_time = 0;
    game_state->player_dead = false;
    game_state->paused = false;
    
    if(!game_state->save.has_seen_tutorial)
    {
        game_state->in_tutorial = true;
        game_state->tutorial_phase = 0;
    }
}

Void
update_main_menu(GameState *game_state)
{
    F32 d_time = game_state->d_time;
    Keys *keys = &game_state->input.keys;
    
    if(keys->down.just_pressed || keys->s.just_pressed)
        game_state->main_menu_selector++;
    if(keys->up.just_pressed || keys->w.just_pressed)
        game_state->main_menu_selector--;
    game_state->main_menu_selector = clamp_circle(game_state->main_menu_selector, 0, 1);
    
    if(keys->enter.just_pressed)
    {
        if(game_state->main_menu_selector == 0)
        {
            start_game(game_state);
        }
        else if(game_state->main_menu_selector == 1)
        {
            game_state->should_quit = true;
        }
        else if(game_state->main_menu_selector == 2)
        {
        }
    }
}

Void
exit_to_main_menu(GameState *game_state)
{
    game_state->in_game = false;
    game_state->main_menu_selector = 0;
    if(game_state->kills > game_state->save.highest_kills)
        game_state->save.highest_kills = game_state->kills;
    if(game_state->time_in_game > game_state->save.highest_time)
        game_state->save.highest_time = game_state->time_in_game;
    
    write_to_save_file(game_state);
}

Void
update_gameplay(GameState *game_state)
{
    F32 d_time = game_state->d_time;
    Player *player = &game_state->player;
    Keys *keys = &game_state->input.keys;
    V2 playing_area_dim = game_state->field.playing_area_dim;
    
    if(!game_state->in_tutorial && !game_state->player_dead)
    {
        game_state->time_in_game += d_time;
    }
    
    if(game_state->in_tutorial)
    {
        if(keys->enter.just_pressed)
        {
            if(game_state->tutorial_phase != 2)
            {
                game_state->tutorial_phase++;
                if(game_state->tutorial_phase == 2)
                {
                    game_state->enemies.add(create_enemy(v2(4, 1), EnemyType::stream));
                    game_state->enemies.add(create_enemy(v2(4, -1), EnemyType::stream));
                }
                
                if(game_state->tutorial_phase >= 4)
                {
                    game_state->in_tutorial = false;
                    game_state->save.has_seen_tutorial = true;
                    start_game(game_state);
                }
            }
        }
    }
    
    if(game_state->tutorial_phase == 2)
    {
        if(game_state->enemies.length == 0)
        {
            game_state->tutorial_phase++;
        }
    }
    
    if(game_state->player_dead)
    {
        if(keys->enter.just_pressed)
            exit_to_main_menu(game_state);
    }
    
    Float player_speed = 5.0f;
    if(keys->shift_left.is_down)
        player_speed = 2.5f;
    
    if(!game_state->player_dead)
    {
        if(keys->d.is_down)
            player->pos.x += player_speed * d_time;
        if(keys->a.is_down)
            player->pos.x -= player_speed * d_time;
        if(keys->w.is_down)
            player->pos.y -= player_speed * d_time;
        if(keys->s.is_down)
            player->pos.y += player_speed * d_time;
        
        if(player->shot_cooldown <= 0)
        {
            V2 bullet_dir = v2(0, 0);
            Bool should_shoot = false;
            if(keys->up.is_down)
            {
                bullet_dir += v2(0, -1);
                should_shoot = true;
            }
            if(keys->down.is_down)
            {
                bullet_dir += v2(0, 1);
                should_shoot = true;
            }
            if(keys->left.is_down)
            {
                bullet_dir += v2(-1, 0);
                should_shoot = true;
            }
            if(keys->right.is_down)
            {
                bullet_dir += v2(1, 0);
                should_shoot = true;
            }
            bullet_dir = bullet_dir.normalized();
            
            if(should_shoot)
            {
                V2 bullet_vel = bullet_dir * 10.0f;
                player->shot_cooldown = player->shot_cooldown_max;
                game_state->player_bullets.add(bullet(player->pos, bullet_vel,
                                                      0.15f, color(0.88f, 0.42f, 0.88f, 1.0f)));
                
            }
        }
        else
        {
            player->shot_cooldown -= d_time;
        }
    }
    
    V2 play_area_top_left = game_state->field.playing_area_dim / -2;
    V2 play_area_bottom_right = game_state->field.playing_area_dim / 2;
    player->pos = clamp(player->pos, play_area_top_left, play_area_bottom_right);
    
    EnemyType new_enemy_type = EnemyType::none;
    if(keys->number_1.just_pressed)
        new_enemy_type = EnemyType::spread;
    if(keys->number_2.just_pressed)
        new_enemy_type = EnemyType::stream;
    if(keys->number_3.just_pressed)
        new_enemy_type = EnemyType::spin;
    if(keys->number_4.just_pressed)
        new_enemy_type = EnemyType::wall;
    if(keys->number_5.just_pressed)
        new_enemy_type = EnemyType::bomb;
    if(keys->number_6.just_pressed)
        new_enemy_type = EnemyType::suicide;
    
    if(new_enemy_type != EnemyType::none)
    {
        Float enemy_fire_time = 0.3f;
        V2 enemy_pos = v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
                          random_float(-playing_area_dim.y/2, playing_area_dim.y/2));
        
        Enemy new_enemy = create_enemy(enemy_pos, new_enemy_type);
        game_state->enemies.add(new_enemy);
    }
    
#if 0
    if(keys->k.just_pressed)
    {
        for(Int i = 0; i < game_state->enemies.length; i++)
        {
            game_state->enemies.remove_at(i);
            i--;
        }
        for(Int i = 0; i < game_state->enemy_bullets.length; i++)
        {
            game_state->enemy_bullets.remove_at(i);
            i--;
        }
    }
#endif
    
    if(game_state->time_in_game - game_state->last_spawn_time >= 1 &&
       !game_state->in_tutorial && !game_state->player_dead)
    {
        game_state->last_spawn_time = game_state->time_in_game;
        /* costs:
stream: 5
suicide: 10
spin: 30
wall: 50
bomb: 60
*/
        // TODO: initial enemy spawn wave (before 5 seconds)
        // TODO: penalty for not killing enemies. Maybe they get stronger the longer they're alive?
        // TODO: enemy spawn queue?
        // TODO: make sure enemies don't spawn right next to or on top of the player
        
        Int max_enemies = 3;
        if(game_state->time_in_game > 30)
            max_enemies = 5;
        if(game_state->time_in_game > 60)
            max_enemies = 7;
        if(game_state->time_in_game > 120)
            max_enemies = 10;
        if(game_state->time_in_game > 180)
            max_enemies = 12;
        
        Int new_enemies_count = max_enemies - game_state->enemies.length;
        if(new_enemies_count > 0)
        {
            Float spawning_preference = sin(game_state->time_in_game * 0.2f);
            
            Float proportions[5] = {
                random_float(0, 0.4),
                random_float(0, 0.3),
                random_float(0, 0.1),
                random_float(0, 0.1),
                random_float(0, 0.1),
            };
            
            EnemyType types[5] = {
                EnemyType::stream,
                EnemyType::suicide,
                EnemyType::spin,
                EnemyType::wall,
                EnemyType::bomb,
            };
            
            Int type_offset = (Int)((spawning_preference+1)/2*4);
            
            Float *enemy_types = (Float *)mem_alloc(sizeof(Float) * new_enemies_count);
            
            for(Int i = 0; i < new_enemies_count; i++)
            {
                Float cost = 1.0f / (Float)new_enemies_count;
                Int largest_index = 0;
                for(Int a = 1; a < 5; a++)
                {
                    if(proportions[a] > proportions[largest_index])
                        largest_index = a;
                }
                
                proportions[largest_index] -= cost;
                
                Float player_safety_radius = 1.5f;
                V2 enemy_pos = v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
                                  random_float(-playing_area_dim.y/2, playing_area_dim.y/2));
                Float dist_to_player = v2_dist(player->pos, enemy_pos);
                if(dist_to_player <= player_safety_radius)
                {
                    V2 travel_dir = enemy_pos - player->pos;
                    travel_dir = travel_dir.normalized();
                    travel_dir = travel_dir * dist_to_player;
                    enemy_pos += travel_dir;
                    if(enemy_pos.x < -playing_area_dim.x/2 || enemy_pos.y < -playing_area_dim.y/2 ||
                       enemy_pos.x > playing_area_dim.x/2 || enemy_pos.y > playing_area_dim.y/2)
                    {
                        enemy_pos = v2(random_float(-0.5, 0.5), random_float(-0.5, 0.5));
                    }
                }
                
                
                Enemy new_enemy = create_enemy(enemy_pos, types[(largest_index + type_offset)%5]);
                game_state->enemies.add(new_enemy);
            }
            
            free(enemy_types);
        }
    }
    
    for(Int i = 0; i < game_state->enemies.length; i++)
    {
        Enemy *enemy = &(game_state->enemies.data[i]);
        
        Bool enemy_should_be_destroyed = false;
        for(Int a = 0; a < game_state->player_bullets.length; a++)
        {
            Bullet *bullet = &game_state->player_bullets.data[a];
            if(v2_dist(enemy->pos, bullet->pos) <= enemy->radius + bullet->radius)
            {
                game_state->player_bullets.remove_at(a);
                enemy_should_be_destroyed = true;
                if(!game_state->in_tutorial)
                {
                    game_state->kills++;
                }
                break;
            }
        }
        
        if(game_state->life_lost_explosion_enabled)
        {
            Float dist_to_explosion = v2_dist(game_state->life_lost_explosion_center, enemy->pos);
            if(dist_to_explosion <= game_state->life_lost_explosion_radius + enemy->radius &&
               !game_state->in_tutorial)
            {
                enemy_should_be_destroyed = true;
            }
        }
        
        
        if(enemy_should_be_destroyed)
        {
            game_state->enemy_explosions.add(enemy_explosion(&(game_state->enemies.data[i])));
            game_state->enemies.remove_at(i);
            i--;
            continue;
        }
        
        if(enemy->type == EnemyType::suicide)
        {
            V2 d_pos = player->pos - enemy->pos;
            d_pos = d_pos.normalized();
            d_pos = d_pos * enemy->suicide_move_speed * d_time;
            enemy->pos += d_pos;
        }
        
        if(!game_state->life_lost_explosion_enabled)
        {
            Float dist_to_player = v2_dist(player->pos, enemy->pos);
            if(dist_to_player <= enemy->radius)
            {
                player_subtract_life(game_state);
            }
        }
        
        enemy->time_to_fire -= d_time;
        if(enemy->time_to_fire <= 0)
        {
            enemy->time_to_fire = enemy->time_between_fires;
            Float bullet_size = 0.2f;
            
            switch(enemy->type)
            {
                case EnemyType::spread:
                {
                    for(Int a = 1; a <= enemy->amount_per_spread; a++)
                    {
                        Float angle = ((Float)a / (Float)enemy->amount_per_spread) * pi*2;
                        V2 bullet_dir = v2(cos(angle), sin(angle));
                        game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                             bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    }
                }; break;
                case EnemyType::stream:
                {
                    V2 bullet_dir = player->pos - enemy->pos;
                    bullet_dir = bullet_dir.normalized();
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    
                }; break;
                case EnemyType::spin:
                {
                    Float initial_angle = get_time() * pi*2 * enemy->spin_speed;
                    for(Int a = 0; a < enemy->spin_arm_count; a++)
                    {
                        Float angle = initial_angle + ((Float)a / (Float)enemy->spin_arm_count) * pi*2;
                        V2 bullet_dir = v2(cos(angle), sin(angle));
                        game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                             bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    }
                }; break;
                case EnemyType::wall:
                {
                    V2 bullet_dir = enemy->wall_dir;
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    bullet_dir = enemy->wall_dir * -1;
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                }; break;
                case EnemyType::bomb:
                {
                    V2 bullet_dir = player->pos - enemy->pos;
                    Float wobble_amount = 0.2f;
                    V2 wobble = v2(random_float(-wobble_amount, wobble_amount), random_float(-wobble_amount, wobble_amount));
                    bullet_dir = bullet_dir.normalized();
                    bullet_dir += wobble;
                    bullet_dir = bullet_dir.normalized();
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size * 4.5f, color(1.0f, 0.8f, 0.0f, 1.0f)));
                }; break;
            }
        }
    }
    
    if(game_state->life_lost_explosion_enabled)
    {
        game_state->life_lost_explosion_radius += d_time * 10;
        if(game_state->life_lost_explosion_radius >= 13)
        {
            game_state->life_lost_explosion_enabled = false;
            game_state->life_lost_explosion_radius = 0;
        }
    }
    
    for(Int i = 0; i < game_state->enemy_bullets.length; i++)
    {
        Bullet *bullet = &game_state->enemy_bullets.data[i];
        
        if(!game_state->life_lost_explosion_enabled)
        {
            Float dist_to_player = v2_dist(player->pos, bullet->pos);
            if(dist_to_player <= bullet->radius)
            {
                player_subtract_life(game_state);
            }
        }
        
        if(game_state->life_lost_explosion_enabled)
        {
            Float dist_to_explosion = v2_dist(game_state->life_lost_explosion_center, bullet->pos);
            if(dist_to_explosion <= game_state->life_lost_explosion_radius + bullet->radius)
            {
                game_state->enemy_bullets.remove_at(i);
                i--;
                continue;
            }
        }
        
        // Later: make * function for V2 and Float
        //bullet->pos += v2(bullet->vel.x * d_time, bullet->vel.y * d_time);
        bullet->pos += bullet->vel * d_time;
        if(bullet->pos.x < -playing_area_dim.x/2 - bullet->radius ||
           bullet->pos.x > playing_area_dim.x/2 + bullet->radius || 
           bullet->pos.y < -playing_area_dim.y/2 - bullet->radius ||
           bullet->pos.y > playing_area_dim.y/2 + bullet->radius)
        {
            game_state->enemy_bullets.remove_at(i);
            i--;
            continue;
        }
    }
    
    for(Int i = 0; i < game_state->player_bullets.length; i++)
    {
        Bullet *bullet = &game_state->player_bullets.data[i];
        // Later: make * function for V2 and Float
        bullet->pos += bullet->vel * d_time;
        if(bullet->pos.x < -playing_area_dim.x/2 - bullet->radius ||
           bullet->pos.x > playing_area_dim.x/2 + bullet->radius || 
           bullet->pos.y < -playing_area_dim.y/2 - bullet->radius ||
           bullet->pos.y > playing_area_dim.y/2 + bullet->radius)
        {
            game_state->player_bullets.remove_at(i);
            i--;
        }
    }
    
    for(Int i = 0; i < game_state->enemy_explosions.length; i++)
    {
        EnemyExplosion *explosion = &game_state->enemy_explosions.data[i];
        explosion->time_left -= d_time;
        
        if(explosion->time_left <= 0)
        {
            game_state->enemy_explosions.remove_at(i);
            i--;
        }
    }
}


