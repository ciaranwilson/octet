                                                    INVADERER GEAR SOLID

I intended to create a stealth action game in the style of Metal Gear Solid, by expanding on the basic structure of Invaderers. I wanted to include walls and/or barriers to create spaces for hiding, to provide some interest in the level. (fig.1) 
I also wanted to introduce new mechanics such as the ability to plant bombs to destroy enemy sentries (fig.2) and also a stealth mechanic, whereby the player must remain hidden - to be spotted by an emeny means Game Over, much like Metal Gear played on the harder settings (fig.3).

1. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/game.jpg" height="200px"> 2. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/bomb.jpg" height="200px"> 3. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/spotted.jpg" height="200px"> 4. <img src="https://raw.githubusercontent.com/CiaranWilson/octet/master/octet/assets/invaderers/complete.jpg" height="200px">

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
			
The next major addition was the plant_mine method, which allows the player to sneak up behind the enemy sentry and plant a bomb, which detonates when stepped upon. To do this I created get_Position expression, which gathers and returns the coordinates of the player sprite and tells the program where to plant the mine when implemented in the plant_mine function:

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


Next I added my own fragment shader which I then set to render the mid tone grey colour of the ground, rather than using my initial tiled texture.

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



While testing this mechanic I ran into a problem - because the detection is based on the players proximity to the y axis of any given enemy, and the enemies are translated along the X axis once destroyed, it was causing Game Over to occur when traversing the path of an enemy that had already been 'killed', thus it was necessary to translate the dispatched emeny on both axes.










Below is a game demo that shows the gameplay, and a second video that demonstratesd the stealth mechanic

game demo: https://www.youtube.com/watch?v=-E_BIQWpz9c&feature=youtu.be

failure demo:   https://www.youtube.com/watch?v=T03xZPjurNc&feature=youtu.be




