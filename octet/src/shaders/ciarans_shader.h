namespace octet {
	namespace shaders {
		class ciarans_shader : public shader {

		public:
			void init() {
				const char vertex_shader[] = SHADER_STR(
					varying vec2 uv_;

				attribute vec4 pos;
				attribute vec2 uv;

				uniform mat4 modelToProjection;

				void main() { gl_Position = modelToProjection * pos; uv_ = uv; }
				);


				const char fragment_shader[] = SHADER_STR(
					varying vec2 uv_;
				uniform sampler2D sampler;
				void main() { gl_FragColor = texture2D(sampler, uv_); }
				);

				shader::init(vertex_shader, fragment_shader);
			}
		}
 ;
	}
}