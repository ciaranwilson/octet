namespace octet {
	namespace shaders {
		class ciarans_shader : public shader {
			GLuint modelToProjectionIndex_;

		public:
			void init() {

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

				shader::init(vertex_shader, fragment_shader);

				modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
			}

			void render(const mat4t &modelToProjection) {
				shader::render();
				glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
			}
		};
	}
}
