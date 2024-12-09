
Void
update_camera(GameState *game_state)
{
    Camera *camera = &(game_state->camera);
    Input *input = &(game_state->input);
    Float d_time = game_state->d_time;
    
    if(camera->orbiting)
    {
        Float camera_orbit_speed = 2.0f;
        
#if 0
        // TODO: make this toggleable in debug menu
        if(game_state->input.left_mouse_down)
        {
            Float camera_mouse_pan_orbit_speed = 2.0f;
            camera->orbit_angles.y += input->d_mouse_pos.y * camera_mouse_pan_orbit_speed * d_time;
            camera->orbit_angles.x += input->d_mouse_pos.x * camera_mouse_pan_orbit_speed * d_time;
        }
        
        if(input->d_scroll != 0)
        {
            camera->orbit_distance -= input->d_scroll * 0.5 * (camera->orbit_distance);
        }
#endif
        
        Float angle_y_min = -1*pi/2.2f;
        if(camera->orbit_angles.y < angle_y_min)
            camera->orbit_angles.y = angle_y_min;
        if(camera->orbit_angles.y > -angle_y_min)
            camera->orbit_angles.y = -angle_y_min;
        
        
        camera->target_pos.x = camera->orbit_distance * cos(camera->orbit_angles.y) * cos(camera->orbit_angles.x);
        camera->target_pos.y = camera->orbit_distance * sin(camera->orbit_angles.y);
        camera->target_pos.z = camera->orbit_distance * cos(camera->orbit_angles.y) * sin(camera->orbit_angles.x);
    }
    
    {
        Float interp_speed = 50.0f;
        camera->pos = interpolate(camera->pos, camera->target_pos, interp_speed * d_time);
        camera->target = interpolate(camera->target, camera->target_target, interp_speed * d_time);
#if 0
        real_camera->orbit_angles = interpolate(real_camera->orbit_angles, camera->orbit_angles, interp_speed * d_time);
        real_camera->orbit_distance = interpolate(real_camera->orbit_distance,
                                                  camera->orbit_distance,
                                                  interp_speed * d_time);
#endif
    }
}