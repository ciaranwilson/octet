                                                    INVADERER GEAR SOLID

I intended to create a stealth action game in the style of Metal Gear Solid, by expanding on the basic structure of Invaderers. I wanted to include walls and/or barriers to create spaces for hiding, to provide some interest in the level. (fig.1) 
I also wanted to introduce new mechanics such as the ability to plant bombs to destroy enemy sentries (fig.2) and also a stealth mechanic, whereby the player must remain hidden - to be spotted by an emeny means Game Over, much like Metal Gear played on the harder settings (fig.3).

1. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/game.jpg" height="450px"> 2. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/bomb.jpg" height="450px"> 

The first step was to introduce more movement for the player, I implemented a rotation method within the main in the sprite class, to rotate the players character on the Z axis, using the left and right keys. 

    void rotate(float angle) {
			modelToWorld.rotateZ(angle);
			
This allowed for a tank control system, where the player can move forward and steer left and right simultaneously with the up, down, left and right keys, allowing for 360 degree movement. Next I created 2 new borders to use as walls within the game space. After this I used the collides_with method so the player wont be able to travel out of bounds or through the newly created walls. I also resized the game space to allow for a gretaer range of movement and improve the playing experience.
Next I implemented a Comma Seperated Value file to delineate the walls and ground, doing away with the unnecessary borders.
First I declared map_height and map_width variables, and two dynarrays, for the 'walls' and 'ground'

•	Next I set the walls and floor arrays to render the ground and wall textures I created, then I adapted the collision code to work with CSV walls rather than the original borders:

        if (is_key_down(key_up)) {
				sprites[ship_sprite].translate(0, ship_speed);
				for (unsigned int j = 0; j < map_walls.size(); ++j) {
					if (sprites[ship_sprite].collides_with(map_walls[j])) {
						sprites[ship_sprite].translate(0, -ship_speed);
					}
				}
			}
			
The next major addition was the plant_mine method, which allows the player to sneak up behind the enemy sentry and plant a bomb, which then detonates when stepped upon. To do this I created the get_Position expression, which gathers and returns the coordinates of the player sprite and tells the program where to plant the mine when implemented in plant_mine:

    void plant_mine() {
			vec2 pos = sprites[ship_sprite].get_Position();
			if (is_key_going_down(key_x)) {
				for (int i = 0; i != num_mines; ++i) {
					GLuint mine = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/mine.gif");
					sprites[first_mine_sprite + i].init(mine, pos.x(), pos.y(), 0.25f, 0.25f);
					

							break;
						}
					}
				}


Next I added my own fragment shader which I then set to render the mid tone grey colour of the ground, rather than using my initial tiled texture. I created a shader program called ciarans_shader and included it in the shaders library for use within the game demo. I created the vertex shader which sets the position of each vertex and outputs the results to the fragment shader, which is responsible for colouring the pixels:

    const char vertex_shader[] = SHADER_STR(
					attribute vec4 pos;
				uniform mat4 modelToProjection;

				void main() {
					gl_Position = modelToProjection * pos;
				}
				);

				const char fragment_shader[] = SHADER_STR(

					void main() {
					vec2 p = gl_FragCoord.xy / vec2(750, 750);
					vec3 col = vec3(0.224, 0.224, 0.224);
					gl_FragColor = vec4(col, 1.0);
				}
				);

   Here is where I used ciarans_shader to render the colour information of the ground:
   
   
      for (unsigned int i = 0; i < map_ground.size(); ++i) {
		  map_ground[i].render(ciarans_shader_, cameraToWorld);
	  }
	  
Next I implemented the stealth mechanic, the central and defining element of the game demo. The player must only advance when the enemies' backs are turned, if the player advances while the enemy sentry is facing towards the player, game over instantly occurs (fig.3)
I created bool is_facing_right and set the default value to true.
Then I created a method in the sprite class that returns the necessary information that decides whether or not the player is spotted:

    void check_sight() {
		for (unsigned int i = first_invaderer_sprite; i <= last_invaderer_sprite; ++i) {
			float dy = fabsf(sprites[i].get_Position().y() - sprites[ship_sprite].get_Position().y());
			if (dy < 0.15f) {
				float dx = sprites[ship_sprite].get_Position().x() - sprites[i].get_Position().x();
				if (sprites[i].facing_right() && dx > 0.0f) {
					game_over = true;
					sprites[game_over_sprite].translate(-20, 0);
				}
				else if (!sprites[i].facing_right() && dx < 0.0f) {
					game_over = true;
					sprites[game_over_sprite].translate(-20, 0);
				}
			}
		}
	}

While testing this mechanic I ran into a problem - because the detection is based on the players proximity to the y axis of any given enemy, and the enemies are translated along the X axis once 'destroyed' and therefore remain int the same y coordinate, it was causing Game Over to occur when traversing the path of an enemy that had already been dealt with, thus it was necessary to translate the dispatched enemy along both axes.



In order to make the sentries appear to change the direction theyre facing, I included a function to swap out the sprite for one that i flipped horizontally in Photoshop:

	void flip_inv_sprites() {
		for (unsigned int i = first_invaderer_sprite; i <= last_invaderer_sprite; ++i) {
			if (sprites[i].facing_right()) {
				GLuint trooper = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/trooper_right.gif");
				sprites[i].swap_texture(trooper);
				
			}
			else {
				GLuint trooper = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/trooper.gif");
				sprites[i].swap_texture(trooper);
				
			}
			
		}
		
	}

3 <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/spotted.jpg" height="450px"> 4. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/complete.jpg" height="450px">

When only one enemy remains, it triggers alert mode, causing a red filter to appear on the screen and the remaining enemy to speed up and actively seek out the player (fig.3)

    if (live_invaderers == 1) {
				invader_velocity *= 2;
				
    if (live_invaderers == 1) {
						move_invaders(invader_velocity, -0.25f);
					}

When the final enemy has been killed, the player has completed the level and is greeted with a congratulary message (fig.4)




Find below the demo video of my game and a second video demonstrating the stealth mechanic in action:



game demo: https://www.youtube.com/watch?v=-E_BIQWpz9c&feature=youtu.be

failure demo:   https://www.youtube.com/watch?v=T03xZPjurNc&feature=youtu.be




