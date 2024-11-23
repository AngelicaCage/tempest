
Void
draw_axes(GameState *game_state)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Int positive_line_width = 3;
    Int negative_line_width = 1;
    
    Float line_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    Int color_loc = glGetUniformLocation(game_state->shader_programs[1].id, "lineColor");
    glm::mat4 model = glm::mat4(1.0f);
    Int model_loc = glGetUniformLocation(game_state->shader_programs[1].id, "model");
    glUseProgram(game_state->shader_programs[1].id);
    glBindVertexArray(game_state->axis_vao);
    
    { // x axis
        glUniform4fv(color_loc, 1, line_color);
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    { // z axis
        line_color[0] = 0.0f;
        line_color[2] = 1.0f;
        glUniform4fv(color_loc, 1, line_color);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    { // y axis
        line_color[2] = 0.0f;
        line_color[1] = 1.0f;
        glUniform4fv(color_loc, 1, line_color);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
}

Void
draw_field(GameState *game_state)
{
    Field *field = &game_state->field;
    // Draw field
    glm::mat4 model = glm::mat4(1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(game_state->shader_programs[0].id);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    Int model_loc = glGetUniformLocation(game_state->shader_programs[0].id, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    
    Int ambient_light_color_loc = glGetUniformLocation(game_state->shader_programs[0].id, "ambientLightColor");
    Int ambient_light_strength_loc = glGetUniformLocation(game_state->shader_programs[0].id, "ambientLightStrength");
    glUniform3f(ambient_light_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform1f(ambient_light_strength_loc, 0.5f);
    
    Int sun_light_color_loc = glGetUniformLocation(game_state->shader_programs[0].id, "sunLightColor");
    Int sun_light_strength_loc = glGetUniformLocation(game_state->shader_programs[0].id, "sunLightStrength");
    Int sun_light_dir_loc = glGetUniformLocation(game_state->shader_programs[0].id, "sunLightDirection");
    glUniform3f(sun_light_color_loc, 1.0f, 1.0f, 1.0f);
    glUniform1f(sun_light_strength_loc, 1.0f);
    glUniform3f(sun_light_dir_loc, -1.0f, -1.0f, 1.0f);
    
    glBindVertexArray(field->vao);
    for(Int i = 0; i < field->height-1; i++)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field->ebos[i]);
        glDrawElements(GL_TRIANGLES, (field->width-1)*6, GL_UNSIGNED_INT, (Void *)0);
    }
    
    
    // Field outlines
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0f));
    model_loc = glGetUniformLocation(game_state->shader_programs[0].id, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(field->vao);
    for(Int i = 0; i < field->height-1; i++)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field->ebos[i]);
        glDrawElements(GL_TRIANGLES, (field->width-1)*6, GL_UNSIGNED_INT, (Void *)0);
    }
    
}