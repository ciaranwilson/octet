////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// INVADERER GEAR SOLID
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//

namespace octet {
	class sprite {
		// where is our sprite (overkill for a 2D game!)
		mat4t modelToWorld;

		// half the width of the sprite
		float halfWidth;

		// half the height of the sprite
		float halfHeight;

		// what texture is on our sprite
		int texture;

		// true if this sprite is enabled.
		bool enabled;

		bool is_facing_right = true;

	public:
		sprite() {
			texture = 0;
			enabled = true;	
		}

		vec2 get_Position() {
			return modelToWorld.row(3).xy();
		}

		//Create a position value
		void set_Position(vec2 pos) {		
			modelToWorld.translate(vec3(pos.x(), pos.y(), 0.0f) - modelToWorld.row(3).xyz());
		}

		void init(int _texture, float x, float y, float w, float h) {
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			halfWidth = w * 0.5f;
			halfHeight = h * 0.5f;
			texture = _texture;
			enabled = true;
		}

		void render(texture_shader &shader, mat4t &cameraToWorld) {
			// invisible sprite... used for gameplay.
			if (!texture) return;

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// use "old skool" rendering
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			shader.render(modelToProjection, 0);

			// this is an array of the positions of the corners of the sprite in 3D
			// a straight "float" here means this array is being generated here at runtime.
			float vertices[] = {
			  -halfWidth, -halfHeight, 0,
			   halfWidth, -halfHeight, 0,
			   halfWidth,  halfHeight, 0,
			  -halfWidth,  halfHeight, 0,
			};

			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			// this is an array of the positions of the corners of the texture in 2D
			static const float uvs[] = {
			   0,  0,
			   1,  0,
			   1,  1,
			   0,  1,
			};

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			// finally, draw the sprite (4 vertices)
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}

		void render(ciarans_shader &shader, mat4t &cameraToWorld) {
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
			shader.render(modelToProjection);

			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth, halfHeight, 0,
				-halfWidth, halfHeight, 0,
			};

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			static const float uvs[] = {
				0, 0,
				1, 0,
				1, 1,
				0, 1,
			};

			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		
		void swap_texture(int tex) {
			texture = tex;
		}

		// move the object
		void translate(float x, float y) {
			modelToWorld.translate(x, y, 0);
		}

		//swap sprite
		void rotate(float angle, float x, float y, float z) {
			modelToWorld.rotate(angle, x, y, z);
		}

		void rotate(float angle) {
			modelToWorld.rotateZ(angle);
		}

		// position the object relative to another.
		void set_relative(sprite &rhs, float x, float y) {
			modelToWorld = rhs.modelToWorld;
			modelToWorld.translate(x, y, 0);
		}

		// return true if this sprite collides with another.
		// note the "const"s which say we do not modify either sprite
		bool collides_with(const sprite &rhs) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
			float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

			// both distances have to be under the sum of the halfwidths
			// for a collision
			return
				(fabsf(dx) < halfWidth + rhs.halfWidth) &&
				(fabsf(dy) < halfHeight + rhs.halfHeight)
				;
		}

		bool is_above(const sprite &rhs, float margin) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

			return
				(fabsf(dx) < halfWidth + margin)
				;
		}

		bool &is_enabled() {
			return enabled;
		}

		bool& facing_right() {
			return is_facing_right;
		}
	};

	class invaderers_app : public octet::app {
		// Matrix to transform points in our camera space to the world.
		// This lets us move our camera
		mat4t cameraToWorld;

		// shader to draw a textured triangle
		texture_shader texture_shader_;
		ciarans_shader ciarans_shader_;

		enum {
			num_sound_sources = 2,
			num_rows = 3,
			num_cols = 1,
			num_missiles = 2,
			num_mines = 1,
			num_borders = 7,
			num_invaderers = num_rows * num_cols,

			// sprite definitions
			ship_sprite = 0,
			
			game_over_sprite,
			
			game_complete_sprite,

			first_invaderer_sprite,
			last_invaderer_sprite = first_invaderer_sprite + num_invaderers - 1,

			first_missile_sprite,
			last_missile_sprite = first_missile_sprite + num_missiles - 1,

			first_mine_sprite,
			last_mine_sprite = first_mine_sprite + num_mines - 1, // Remember to optimize

			first_border_sprite,
			last_border_sprite = first_border_sprite + num_borders - 1,

			alert_sprite,

			num_sprites,

		};
		
		// declaring variables and arrays for CSV file
		
		static const int map_width = 26;
		static const int map_height = 26;
		int map[map_height][map_width];
		dynarray<sprite>map_walls;
		dynarray<sprite>map_ground;

		// timers for missiles and bombs
		int missiles_disabled;
		int bombs_disabled;

		// accounting for bad guys
		int live_invaderers;
		int num_lives;

		// game state
		bool game_over;
		bool game_complete;
		int score;

		// speed of enemy
		float invader_velocity;

		// sounds
		ALuint shot;
		ALuint bang;
		unsigned cur_source;
		ALuint sources[num_sound_sources];

		// big array of sprites
		sprite sprites[num_sprites];

		sprite mines[num_mines];

		// random number generator
		class random randomizer;

		// a texture for our text
		GLuint font_texture;

		// information for our text
		bitmap_font font;

		ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

		// called when we hit an enemy
		void on_hit_invaderer() {

			live_invaderers--;
			score++;
			if (live_invaderers == 1) {
				invader_velocity *= 2;
				sprites[alert_sprite].is_enabled() = true;
			}
			else if (live_invaderers == 0) {
				game_complete = true;
				sprites[game_complete_sprite].translate(-20, 0);
			}
		}

		// called when we are hit
		void on_hit_ship() {
			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, bang);
			alSourcePlay(source);

			if (--num_lives == 0) {
				game_over = true;
				sprites[game_over_sprite].translate(-20, 0);
			}
		}

		// use the keyboard to move the ship
		void move_ship() {
			const float ship_speed = 0.1f;

			// left and right arrows


			if (is_key_down(key_up)) {
				sprites[ship_sprite].translate(0, ship_speed);
				for (unsigned int j = 0; j < map_walls.size(); ++j) {
					if (sprites[ship_sprite].collides_with(map_walls[j])) {
						sprites[ship_sprite].translate(0, -ship_speed);
					}
				}
			}

			if (is_key_down(key_down)) {
				sprites[ship_sprite].translate(0, -ship_speed);
				for (unsigned int j = 0; j < map_walls.size(); ++j) {
					if (sprites[ship_sprite].collides_with(map_walls[j])) {
						sprites[ship_sprite].translate(0, ship_speed);
					}
				}

			}
		}

		//use the keyboard to rotate the ship
		void rotate_ship() {
			const float ship_rotation = 5.0f;
			if (is_key_down(key_left)) {
				sprites[ship_sprite].rotate(ship_rotation);
			}
			else if (is_key_down(key_right)) {
				sprites[ship_sprite].rotate(-ship_rotation);
			}
		}

		// fire button (space)
		void fire_missiles() {
			if (missiles_disabled) {
				--missiles_disabled;
			}
			else if (is_key_going_down(' ')) {
				// find a missile
				for (int i = 0; i != num_missiles; ++i) {
					if (!sprites[first_missile_sprite + i].is_enabled()) {
						sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0, 0.5f);
						sprites[first_missile_sprite + i].is_enabled() = true;
						missiles_disabled = 5;
						ALuint source = get_sound_source();
						alSourcei(source, AL_BUFFER, shot);
						alSourcePlay(source);
						break;
					}
				}
			}
		}
	
		// animate the missiles
		void move_missiles() {
			const float missile_speed = 0.3f;
			for (int i = 0; i != num_missiles; ++i) {
				sprite &missile = sprites[first_missile_sprite + i];
				if (missile.is_enabled()) {
					missile.translate(0, missile_speed);
					for (int j = 0; j != num_invaderers; ++j) {
						sprite &invaderer = sprites[first_invaderer_sprite + j];
						if (invaderer.is_enabled() && missile.collides_with(invaderer)) {
							invaderer.is_enabled() = false;
							invaderer.translate(20, 20);
							missile.is_enabled() = false;
							missile.translate(20, 0);
							on_hit_invaderer();

							goto next_missile;
						}
					}

					for (unsigned int j = 0; j < map_walls.size(); ++j) {

						if (missile.collides_with(map_walls[j])) {
							missile.is_enabled() = false;
							missile.translate(20, 0);
						}
					}
				}
			next_missile:;
			}
		}

		// plant a mine to dispatch enemies
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
	
		// move the array of enemies
		void move_invaders(float dx, float dy) {
			for (int j = 0; j != num_invaderers; ++j) {
				sprite &invaderer = sprites[first_invaderer_sprite + j];
			if (invaderer.is_enabled()) {
				invaderer.translate(dx, dy);
			for (int i = 0; i != num_mines; ++i) {
				sprite &mines = sprites[first_mine_sprite + i];			
					if (invaderer.is_enabled() && invaderer.collides_with(mines)) {
						invaderer.is_enabled() = false;
						invaderer.translate(20, 20);
						mines.is_enabled() = false;
						mines.translate(20, 0);
						ALuint source = get_sound_source();
						alSourcei(source, AL_BUFFER, bang);
						alSourcePlay(source);
						on_hit_invaderer();
						}
					}
				}
			}
		}
		
		// check if any enemies hit the sides.
		bool invaders_collide(sprite &border) {
      for (int j = 0; j != num_invaderers; ++j) {
        sprite &invaderer = sprites[first_invaderer_sprite+j];
        if (invaderer.is_enabled() && invaderer.collides_with(border)) {
          return true;
        }
      }
      return false;
    }

		void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.scale(scale, scale, 1);
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      /*mat4t tmp;
      glLoadIdentity();
      glTranslatef(x, y, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
      glScalef(scale, scale, 1);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

      enum { max_quads = 32 };
      bitmap_font::vertex vertices[max_quads*4];
      uint32_t indices[max_quads*6];
      aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

      unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, font_texture);

      shader.render(modelToProjection, 0);

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x );
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u );
      glEnableVertexAttribArray(attribute_uv);

      glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
    }

  public:

    // this is called when we construct the class
    invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this is called once OpenGL is initialized
    void app_init() {
    
	  // set up the shader
      texture_shader_.init();
	  ciarans_shader_.init();
	  read_csv();
	  setup_visual_map();


      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 4);

      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

      GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/soldier_2.gif");
      sprites[ship_sprite].init(ship, -3, -3.3f, 0.25f, 0.5f);

	  sprites[ship_sprite].rotate(-90);

      GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
      sprites[game_over_sprite].init(GameOver, 20, 0, 6, 3);

	  GLuint GameComplete = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameComplete.gif");
	  sprites[game_complete_sprite].init(GameComplete, 20, 0, 6, 3);

      GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/trooper_right.gif");
      for (int j = 0; j != num_rows; ++j) {
        for (int i = 0; i != num_cols; ++i) {
          assert(first_invaderer_sprite + i + j*num_cols <= last_invaderer_sprite);
          sprites[first_invaderer_sprite + i + j*num_cols].init(
            invaderer, ((float)i - num_cols * 0.5f) * 0.5f, 2.50f - ((float)j * 2.2f), 0.5f, 0.5f
          );
        }
      }

	  GLuint red = resource_dict::get_texture_handle(GL_RGBA, "#cc221144");
	  sprites[alert_sprite].init(red, 0, 0, 8, 8);
	  sprites[alert_sprite].is_enabled() = false;

      // use the missile texture
      GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile_2.gif");
      for (int i = 0; i != num_missiles; ++i) {
        // create missiles off-screen
        sprites[first_missile_sprite+i].init(missile, 20, 0, 0.0625f, 0.25f);
        sprites[first_missile_sprite+i].is_enabled() = false;
      }

	  //GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
	  //GLuint bg = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/spider_background.gif");
	 // backgroud_sprite.init(0, 1, 1, 3, 3);

      // sounds
      shot = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/shot.wav");
      bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      // sundry counters and game state.
      missiles_disabled = 0;
      invader_velocity = 0.03f;
      live_invaderers = num_invaderers;
	  game_over = false;
	  game_complete = false;
      score = 0;
	  flip_inv_sprites();
    }

	// this is to check the enemies' line of sight for the player
	void check_sight() {
		for (unsigned int i = first_invaderer_sprite; i <= last_invaderer_sprite; ++i) {
			float dy = fabsf(sprites[i].get_Position().y() - sprites[ship_sprite].get_Position().y());
			if (dy < 0.8f) {
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
	
	// this swaps the enemy sprite when it changes direction
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
	
    // called every frame to move things
	void simulate() {
		if (game_over) {
			return;					
		}
		if (game_complete) {
			return;
		}

		move_ship();

		rotate_ship();

		fire_missiles();

		plant_mine();

		move_missiles();

		move_invaders(invader_velocity, 0);

		check_sight();

		for (unsigned int i = 0; i < map_walls.size(); i = i + 2) {

			sprite &border = map_walls[(invader_velocity < 0 ? (20 + i) : (19 + i))];
			if (invaders_collide(border)) {

				invader_velocity = -invader_velocity;
				for (unsigned int j = first_invaderer_sprite; j <= last_invaderer_sprite; ++j) {
					sprites[j].facing_right() = !sprites[j].facing_right();
					flip_inv_sprites();

					if (live_invaderers == 1) {
						move_invaders(invader_velocity, -0.25f);
					}
				}
			}
		}
			
	}
    // this is called to draw the world
    
	void draw_world(int x, int y, int w, int h) {
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);


      // clear the background to black
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	  // drawing background
	/*  backgound_sprite.render(ciarans_shader_, cameraToWorld, w, h);
	  backgroud_sprite.translate(1, 1);*/

	  for (unsigned int i = 0; i < map_ground.size(); ++i) {
		  map_ground[i].render(ciarans_shader_, cameraToWorld);
	  }
	
	  // draw ground 
	  //draw walls
	  for (unsigned int i = 0; i < map_walls.size(); ++i) {
		  map_walls[i].render(texture_shader_, cameraToWorld);
	  }

	  
	  
	  // draw all the sprites
      for (int i = 0; i != num_sprites; ++i) {
		  if (sprites[i].is_enabled())
			sprites[i].render(texture_shader_, cameraToWorld);
      }

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }
	
	void read_csv() {

		std::ifstream file("ciarans_map.csv");

		char buffer[2048];
		int i = 0;

		while (!file.eof()) {
			file.getline(buffer, sizeof(buffer));

			char *b = buffer;
			for (int j = 0; ; ++j) {
				char *e = b;
				while (*e != 0 && *e != ';') ++e;

				map[i][j] = std::atoi(b);

				if (*e != ';') break;
				b = e + 1;
			}
			++i;
		}
	}

	void setup_visual_map() {

		GLuint walls = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/walls.gif");
		GLuint ground = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ground.gif");

		for (int i = 0; i < map_height; ++i) {
			for (int j = 0; j < map_width; ++j) {

				sprite s;

				if (map[i][j] == 1) {

					s.init(walls, -4 + 0.2f + 0.4f*j, 4 - 0.2f - 0.4f*i, 0.4f, 0.4f);
					map_walls.push_back(s);
				}

				else if (map[i][j] == 0) {

					s.init(walls, -4 + 0.2f + 0.4f*j, 4 - 0.2f - 0.4f*i, 0.4f, 0.4f);
					map_ground.push_back(s);
				
				}
			 }
		 }
	  }
   };
}
