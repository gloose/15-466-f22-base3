#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
//#include "../nest-libs/windows/glm/include/glm/gtc/type_ptr.hpp"
#include <glm/gtc/quaternion.hpp>
//#include "../nest-libs/windows/glm/include/glm/gtc/quaternion.hpp"
#include <glm/gtx/color_space.hpp>
//#include "../nest-libs/windows/glm/include/glm/gtx/color_space.hpp"
#include <random>
#include <iomanip>
#include <sstream>

GLuint meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("game3.pnct"));
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > game3_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("game3.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){});
});

Load< Sound::Sample > beat_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("beat.opus"));
});

Load< Sound::Sample > note_11(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("11.opus")); });
Load< Sound::Sample > note_21(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("21.opus")); });
Load< Sound::Sample > note_31(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("31.opus")); });
Load< Sound::Sample > note_41(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("41.opus")); });
Load< Sound::Sample > note_12(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("12.opus")); });
Load< Sound::Sample > note_22(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("22.opus")); });
Load< Sound::Sample > note_32(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("32.opus")); });
Load< Sound::Sample > note_42(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("42.opus")); });
Load< Sound::Sample > note_13(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("13.opus")); });
Load< Sound::Sample > note_23(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("23.opus")); });
Load< Sound::Sample > note_33(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("33.opus")); });
Load< Sound::Sample > note_43(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("43.opus")); });
Load< Sound::Sample > note_14(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("14.opus")); });
Load< Sound::Sample > note_24(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("24.opus")); });
Load< Sound::Sample > note_34(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("34.opus")); });
Load< Sound::Sample > note_44(LoadTagDefault, []() -> Sound::Sample const* { return new Sound::Sample(data_path("44.opus")); });

std::array<Load<Sound::Sample>*, 16> note_sounds = {
	//&left_sample, &down_sample, &up_sample, &right_sample
	&note_11, &note_12, &note_13, &note_14,
	&note_21, &note_22, &note_23, &note_24,
	&note_31, &note_32, &note_33, &note_34,
	&note_41, &note_42, &note_43, &note_44
};

void PlayMode::setMesh(Scene::Drawable* drawable, std::string mesh_name) {
	// Assign the mesh with the given name to the given drawable
	Mesh const& mesh = meshes->lookup(mesh_name);
	drawable->pipeline = lit_color_texture_program_pipeline;
	drawable->pipeline.vao = meshes_for_lit_color_texture_program;
	drawable->pipeline.type = mesh.type;
	drawable->pipeline.start = mesh.start;
	drawable->pipeline.count = mesh.count;
}

PlayMode::Note* PlayMode::makeNote(Track* track, Direction dir, bool target) {
	// Initialize note with drawable and transform
	Note* note = new Note();
	note->track = track;
	note->dir = dir;
	scene.drawables.emplace_back(&note->transform);
	note->drawable = &scene.drawables.back();
	note->transform.parent = &track->transform;
	note->transform.position = glm::vec3(0, 0, 0);

	// Add note to track's note vector or target vector
	if (target) {
		track->targets.push_back(note);
	}
	else {
		track->notes.push_back(note);
	}

	// Determine note mesh, rotation, and position based on note direction
	std::string mesh_name = "";
	switch (dir) {
	case dir_left:
		note->transform.position.x = -.1f;
		note->transform.rotation = glm::angleAxis((float)M_PI / 2.f, glm::vec3(0.f, 0.f, 1.f));
		mesh_name = "Note_Left";
		break;
	case dir_down:
		note->transform.position.x = -.033f;
		note->transform.rotation = glm::angleAxis((float)M_PI, glm::vec3(0.f, 0.f, 1.f));
		mesh_name = "Note_Down";
		break;
	case dir_right:
		note->transform.position.x = .033f;
		note->transform.rotation = glm::angleAxis(-(float)M_PI / 2.f, glm::vec3(0.f, 0.f, 1.f));
		mesh_name = "Note_Right";
		break;
	case dir_up:
		note->transform.position.x = .1f;
		note->transform.rotation = glm::angleAxis(0.f, glm::vec3(0.f, 0.f, 1.f));
		mesh_name = "Note_Up";
		break;
	}

	if (target) {
		mesh_name = "Note_Target";
		note->transform.position.y = target_y;
	}

	setMesh(note->drawable, mesh_name);

	return note;
}

void PlayMode::miss(Track* track) {
	active_track = nullptr;
	track->song_offset = 0;
	for (size_t i = 0; i < track->notes.size(); i++) {
		if (track->notes[i] != nullptr) {
			track->notes[i]->hit = false;
			track->notes[i]->transform.position.z = 0;
		}
	}
	for (size_t i = 0; i < track->targets.size(); i++) {
		track->targets[i]->transform.scale = glm::vec3(1.f, 1.f, 1.f);
	}
	can_use_spell = true;
}

void PlayMode::useSpell(size_t spell_index, size_t dir_index, size_t power) {
	//miss(&tracks[spell_index]);

	auto killEnemy = [&](Enemy* enemy) {
		enemy->alive = false;
		glm::vec3 delta = enemy->transform.position - player.transform.position;
		glm::vec3 perp = glm::vec3(delta[1], -delta[0], delta[2]);
		enemy->transform.rotation = glm::angleAxis(-(float)M_PI / 2.f, glm::normalize(perp)) * enemy->transform.rotation;
	};

	switch (spell_index) {
		case water_index:
		{
			// Place water pool
			water.timer = water.min_duration + (float)power * water.time_per_note;
			water.transform.position = player.transform.position + dir_vectors[dir_index] * water.range;
			water.radius = water.min_radius + (float)power * water.size_per_note;
			water.transform.scale = glm::vec3(water.radius, water.radius, 1.f);
			break;
		}
		case earth_index:
		{
			// Start the earth animation
			earth.timer = earth.duration;
			earth.transform.position = player.transform.position;
			float r = earth.min_radius + earth.radius_per_note * (float)power;
			earth.transform.scale = glm::vec3(r, r, 1.f);
			float theta = atan2(dir_vectors[dir_index].y, dir_vectors[dir_index].x);
			earth.transform.rotation = glm::angleAxis(-(float)M_PI / 4.f + theta, glm::vec3(0.f, 0.f, 1.f));

			// Kill nearby enemies
			for (size_t i = 0; i < enemies.size(); i++) {
				Enemy* enemy = enemies[i];
				glm::vec3 delta = enemies[i]->transform.position - player.transform.position;
				float distance = glm::length(delta);
				float enemy_theta = atan2(delta.y, delta.x);
				float theta_diff = abs(enemy_theta - theta);
				if (theta_diff > M_PI) {
					theta_diff = 2 * (float)M_PI - theta_diff;
				}

				if (theta_diff < M_PI / 3.f && distance < r && enemy->alive) {
					killEnemy(enemy);
					tossed_enemies.push_back(enemy);
				}
			}

			break;
		}
		case fire_index:
		{
			// Find the closest enemy in the chosen direction
			float theta = atan2(dir_vectors[dir_index].y, dir_vectors[dir_index].x);
			float min_distance = 2 * ground_radius;
			Enemy* closest = nullptr;
			for (size_t i = 0; i < enemies.size(); i++) {
				Enemy* enemy = enemies[i];
				glm::vec3 delta = enemies[i]->transform.position - player.transform.position;
				float distance = glm::length(delta);
				float enemy_theta = atan2(delta.y, delta.x);
				float theta_diff = abs(enemy_theta - theta);
				if (theta_diff > M_PI) {
					theta_diff = 2 * (float)M_PI - theta_diff;
				}

				if (theta_diff < M_PI / 3.f && distance < min_distance && enemy->alive) {
					min_distance = distance;
					closest = enemy;
				}
			}

			// Shoot that enemy
			if (closest != nullptr) {
				killEnemy(closest);
				shooting = closest;
				shoot_timer = shoot_duration;
			}
			break;
		}
		case wind_index:
		{
			player.transform.position += dir_vectors[dir_index] * (wind_min_distance + (float)power * wind_distance_per_note);
			if (glm::length(player.transform.position) > ground_radius) {
				player.transform.position *= ground_radius / glm::length(player.transform.position);
			}
			break;
		}
	}
}

bool PlayMode::isEnemy(const Scene::Drawable& drawable) {
	return drawable.pipeline.start == enemy_mesh_start;
}

void PlayMode::reset() {
	// Reset score
	score = 0.f;

	// Delete all enemies
	scene.drawables.remove_if(&PlayMode::isEnemy);
	for (size_t i = 0; i < enemies.size(); i++) {
		delete enemies[i];
	}
	enemies.clear();

	// Reset earth chunk used by the Song of Earth
	earth.transform.position.z = 100.f;
	earth.timer = 0.f;
	tossed_enemies.clear();

	// Reset water pool used by the Song of Water
	water.transform.position.z = 100.f;
	water.timer = 0.f;
	water.radius = 0.f;

	// Reset tracks
	for (size_t i = 0; i < tracks.size(); i++) {
		tracks[i].song_offset = 0.f;
	}

	// Reset player
	player.transform.position = glm::vec3(0.f, 0.f, 0.f);
	player.alive = true;

	// Reset miscellaneous variables
	beat = 0.f;
	can_use_spell = true;
	shooting = nullptr;
	active_track = nullptr;
	enemy_timer = 0.f;
}

PlayMode::PlayMode() : scene(*game3_scene) {
	// Set random seed for a different experience every time
	srand((int)time(nullptr));

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// Initialize tracks
	for (size_t i = 0; i < tracks.size(); i++) {
		// Initialize a new track with a new drawable
		scene.drawables.emplace_back(&tracks[i].transform);
		tracks[i].drawable = &scene.drawables.back();
		setMesh(tracks[i].drawable, track_mesh_names[i]);

		auto clipToCamera = [&](glm::vec2 clip) -> glm::vec3 {
			glm::vec4 hom = glm::inverse(camera->make_projection()) * glm::vec4(clip[0], clip[1], 0, 1);
			hom /= hom[3];
			return glm::vec3(hom[0] * aspect, hom[1], hom[2]);
		};

		float padding = 0.05f;
		float width = track_width;
		float height = track_height;

		float x = -1.f + padding + width / 2;
		if (i % 2) {
			x = -x;
		}

		float y = -1.f + padding + height / 2;
		if (i / 2) {
			y = -y;
		}

		glm::vec3 size_camera = clipToCamera(glm::vec2(width, height));
		glm::vec3 pos_camera = clipToCamera(glm::vec2(x, y));

		// Set track transform
		tracks[i].transform.parent = camera->transform;
		tracks[i].transform.scale = glm::vec3(size_camera[0] / width, size_camera[1] / height, size_camera[0] / width);
		tracks[i].transform.position = pos_camera;

		// Add note targets
		for (size_t j = 0; j < 4; j++) {
			makeNote(&tracks[i], directions[(i + j) % 4], true);
		}

		// Add notes
		std::vector<int> song = songs[i];
		for (size_t j = 0; j < song.size(); j++) {
			if (song[j] > 0) {
				makeNote(&tracks[i], directions[song[j] - 1]);
			}
			else {
				tracks[i].notes.push_back(nullptr);
			}
		}
	}

	// Initialize player
	scene.drawables.emplace_back(&player.transform);
	player.drawable = &scene.drawables.back();
	setMesh(player.drawable, "Harp");
	player.transform.scale = glm::vec3(.5f, 0.5f, 0.5f);

	// Initialize ground
	scene.drawables.emplace_back(&ground_transform);
	setMesh(&scene.drawables.back(), "Ground");
	ground_transform.scale = glm::vec3(ground_radius, ground_radius, 1.f);

	// Initialize ocean
	for (size_t i = 0; i < num_waves; i++) {
		scene.transforms.emplace_back();
		scene.drawables.emplace_back(&scene.transforms.back());
		scene.drawables.back().transform->scale = glm::vec3((float)i * wave_spacing, (float)i * wave_spacing, 1.f);
		setMesh(&scene.drawables.back(), "Ocean");
		ocean.push_back(&scene.drawables.back());
	}

	// Initialize earth chunk used by the Song of Earth
	scene.drawables.emplace_back(&earth.transform);
	earth.transform.position.z = 100.f;
	earth.drawable = &scene.drawables.back();
	setMesh(earth.drawable, "Earth");

	// Initialize water pool used by the Song of Water
	scene.drawables.emplace_back(&water.transform);
	water.transform.position.z = 100.f;
	water.drawable = &scene.drawables.back();
	setMesh(water.drawable, "Water");

	// Find enemy mesh for easy lookup in reset
	Mesh const& mesh = meshes->lookup("Enemy");
	enemy_mesh_start = mesh.start;
}

PlayMode::~PlayMode() {
}

PlayMode::Button::Button(Direction dir) : downs(0), pressed(0), dir(dir) {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_r) {
			reset();
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	// Update beat timing
	beat += elapsed;
	if (beat >= beat_length) {
		beat -= beat_length;
		if (player.alive) {
			Sound::play(*beat_sample, 0.25f);
		}
	}

	// Check if anything is playing at the moment
	bool playing = active_track != nullptr;

	// Update tracks
	for (size_t i = 0; i < tracks.size(); i++) {
		Track& track = tracks[i];
		
		// Check for note hits on the active track, or any track if no track is active
		if ((active_track == &track || !playing) && player.alive) {
			// Get the note that should be played right now
			int current_index = (int)round(track.song_offset);
			Note* current_note = nullptr;
			if (current_index >= 0 && current_index < track.notes.size() && track.notes[current_index] != nullptr) {
				current_note = track.notes[current_index];
			}

			if (fabsf(track.song_offset - (float)current_index) < good_window && (beat < beat_length * good_window || beat > beat_length * (1 - good_window))) {
				for (size_t j = 0; j < buttons.size(); j++) {
					if (buttons[j]->downs >= 1) {
						if (current_note == nullptr) {
							if (can_use_spell) {
								useSpell(i, j, current_index);
								Sound::play(**note_sounds[i * 4 + j]);
								buttons[j]->downs--;
								can_use_spell = false;
								break;
							}
						}
						else if (buttons[j]->dir == current_note->dir) {
							current_note->hit = true;
							if (active_track != &track) {
								active_track = &track;
								track.song_offset = beat / beat_length;
								if (beat > beat_length * (1 - good_window)) {
									track.song_offset -= 1.f;
								}
								track.song_offset -= song_speed * elapsed;
							}
							buttons[j]->downs--;
							Sound::play(**note_sounds[i * 4 + j]);
							break;
						}
					}
				}
			}
			else {
				can_use_spell = true;
			}
		}

		// Update the active track
		if (active_track == &track) {
			// Progress song by time elapsed
			track.song_offset += song_speed * elapsed;

			// Check if the previous note was missed
			int prev_index = (int)floor(track.song_offset - good_window);
			if (prev_index >= 0 && prev_index < track.notes.size() && track.notes[prev_index] != nullptr) {
				Note& prev_note = *track.notes[prev_index];
				if (!prev_note.hit) {
					miss(&track);
				}
			}
			else if (prev_index == track.notes.size()) {
				miss(&track);
			}

			// If extra notes were hit, it's a miss
			for (size_t j = 0; j < buttons.size(); j++) {
				if (buttons[j]->downs > 0) {
					miss(&track);
					break;
				}
			}
		}

		// Update target pulse
		float off_beat;
		if (beat < beat_length / 2.f) {
			off_beat = beat / beat_length;
		}
		else {
			off_beat = 1 - beat / beat_length;
		}
		
		for (size_t j = 0; j < track.targets.size(); j++) {
			Note& note = *track.targets[j];
			if (off_beat < good_window && player.alive) {
				float scale = 1 + (1 - off_beat / good_window) * 0.5f;
				note.transform.scale = glm::vec3(scale, scale, scale);
			}
			else {
				note.transform.scale = glm::vec3(1.f, 1.f, 1.f);
			}
		}

		// Update notes
		for (size_t j = 0; j < track.notes.size(); j++) {
			if (track.notes[j] != nullptr) {
				Note& note = *track.notes[j];
				note.transform.position.y = target_y - note_spacing * j + track.song_offset * note_spacing;
				if (note.transform.position.y - note_size / 2.f > track_height / 2.f || note.transform.position.y + note_size / 2.f < -track_height / 2.f) {
					note.transform.position.z = 100.f;
				}
				else {
					note.transform.position.z = 0.f;
				}
			}
		}
	}

	// Update ocean waves
	for (size_t i = 0; i < ocean.size(); i++) {
		ocean[i]->transform->scale -= glm::vec3(elapsed * wave_speed, elapsed * wave_speed, 0);
		if (ocean[i]->transform->scale.x <= 0) {
			ocean[i]->transform->scale = glm::vec3(num_waves * wave_spacing, num_waves * wave_spacing, 1.f);
		}
	}

	// These updates stop once the game ends
	if (player.alive) {
		// Update score
		score += elapsed;

		// Spawn enemies
		enemy_timer -= elapsed;
		if (enemy_timer <= 0) {
			enemy_timer = enemy_interval;
			enemies.push_back(new Enemy());
			Enemy* enemy = enemies.back();
			scene.drawables.emplace_back(&enemy->transform);
			enemy->drawable = &scene.drawables.back();
			setMesh(&scene.drawables.back(), "Enemy");
			float theta = (float)rand() / (float)RAND_MAX * 2 * (float)M_PI;
			enemy->transform.position = ground_radius * glm::vec3(cos(theta), sin(theta), 0.f);
			//enemy->transform.rotation = glm::angleAxis(theta - (float)M_PI / 2.f, glm::vec3(0.f, 0.f, 1.f));
		}

		// Update enemies
		for (size_t i = 0; i < enemies.size(); i++) {
			Enemy* enemy = enemies[i];

			// Chase after the player
			if (enemy->alive) {
				float theta = atan2f(player.transform.position.y - enemy->transform.position.y, player.transform.position.x - enemy->transform.position.x);
				float speed = enemy_speed;
				if (glm::length(enemy->transform.position - water.transform.position) < water.radius) {
					speed *= water.slowdown;
				}
				enemy->transform.position += speed * elapsed * glm::vec3(cos(theta), sin(theta), 0.f);
				enemy->transform.rotation = glm::angleAxis(theta + (float)M_PI / 2.f, glm::vec3(0.f, 0.f, 1.f));

				if (glm::length(enemy->transform.position - player.transform.position) < enemy_kill_radius) {
					player.alive = false;
				}
			}
			else if (enemy->transform.position.z > -.5f) {
				enemy->transform.position.z -= enemy_decay_speed * elapsed;
			}
		}
	}

	// Update fire beam
	if (shooting) {
		shoot_timer -= elapsed;
		if (shoot_timer <= 0) {
			shooting = nullptr;
		}
	}

	// Update earth animation
	if (earth.timer > 0) {
		float t = earth.duration - earth.timer;
		float h = earth.rise_height;
		float a = -4 * h / powf(earth.duration, 2);
		float b = -earth.duration / 2.f;
		float c = h;
		earth.transform.position.z = a * powf(t + b, 2) + c;
		earth.timer -= elapsed;
	}
	else {
		earth.transform.position.z = 100.f;
		for (size_t i = 0; i < tossed_enemies.size(); i++) {
			Enemy* enemy = tossed_enemies[i];
			enemy->transform.position.z = 0.f;
		}
		tossed_enemies.clear();
	}

	// Update enemies tossed by earth
	for (size_t i = 0; i < tossed_enemies.size(); i++) {
		Enemy* enemy = tossed_enemies[i];
		enemy->transform.position.z = earth.transform.position.z * 3.f;
	}

	// Update water pool
	if (water.timer > 0) {
		water.timer -= elapsed;
	}
	else {
		water.transform.position.z = 100.f;
		water.radius = 0.f;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.f, sqrtf(3) / 2.f, -1.f / 2.f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	GL_ERRORS();

	{ // Draw beat lines on tracks
		for (size_t i = 0; i < tracks.size(); i++) {
			glm::mat4 world_to_clip = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local()) * glm::mat4(tracks[i].transform.make_local_to_world());
			DrawLines lines(world_to_clip);
			for (size_t j = 0; j <= track_height / note_spacing + 1; j ++) {
				float y = target_y + (tracks[i].song_offset - floor(tracks[i].song_offset)) * note_spacing - j * note_spacing;
				glm::u8vec4 color(0xff, 0xff, 0xff, 0xff);
				size_t note_index = (size_t)floor(tracks[i].song_offset) + j;
				if ((note_index < tracks[i].notes.size() && tracks[i].notes[note_index] == nullptr) || note_index == tracks[i].notes.size()) {
					float delta = 0.001f;
					float line_radius = 0.005f;
					for (float k = y - line_radius; k <= y + line_radius; k += delta) {
						lines.draw(glm::vec3(-track_width / 2, k, .01f), glm::vec3(track_width / 2, k, .01f), color);
					}
				}
				else {
					lines.draw(glm::vec3(-track_width / 2, y, .01f), glm::vec3(track_width / 2, y, .01f), color);
				}
			}
		}
	}

	// Draw fire beam (adapted from game 2)
	if (shooting != nullptr) {
		glm::mat4 world_to_clip = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local());
		DrawLines lines(world_to_clip);

		// Convert hsv triple to u8vec4 rgb(a), with a always 255
		auto hsvToRgb = [](float h, uint8_t s, uint8_t v) -> glm::u8vec4 {
			h -= (int)floor(h / 360) * 360;
			glm::tvec3<double> rgb = glm::rgbColor(glm::tvec3<double>((double)h, (double)s / 100, (double)v / 100));
			return glm::u8vec4(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255, 255);
		};

		// Draw beam
		float laser_offset = shoot_timer / shoot_duration * 2 * (float)M_PI;
		Scene::Transform t;
		t.position = player.transform.position;
		t.scale = player.transform.scale;
		float angle = atan2(shooting->transform.position.y - player.transform.position.y, shooting->transform.position.x - player.transform.position.x) + (float)M_PI / 2.f;
		t.rotation = glm::angleAxis(angle, glm::vec3(0.f, 0.f, 1.f));
		for (float theta = laser_offset; theta < 2.f * (float)M_PI + laser_offset; theta += 2.f * (float)M_PI / num_beams) {
			glm::vec3 a = t.make_local_to_world() * glm::vec4(cosf(theta) * laser_radius, 0.f, 0.5f + sinf(theta) * laser_radius, 1.f);
			glm::vec3 b = shooting->transform.position + t.make_local_to_world() * glm::vec4(cosf(theta) * laser_radius, 0.f, sinf(theta) * laser_radius / 2, 1.f) - (t.make_local_to_world() * glm::vec4(0.f, 0.f, 0.f, 1.f));
			lines.draw(a, b, hsvToRgb(laser_hue_amplitude * sin((theta - laser_offset) * laser_hue_frequency) + laser_hue_offset, 100, 100));
		}
	}

	{ // Print text (adapted from game 2)
		glDisable(GL_DEPTH_TEST);

		float draw_aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / draw_aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		float H = 0.1f;
		float ofs = 2.0f / drawable_size.y;
		auto drawText = [&](std::string str, float x, float y, glm::u8vec4 color, bool bold = false) {
			int thickness = 1;
			if (bold) {
				thickness = 3;
			}
			for (int i = -thickness; i <= thickness; i++) {
				for (int j = -thickness; j <= thickness; j++) {
					lines.draw_text(str,
						glm::vec3(-draw_aspect + x * H + ofs * j, -1.0 + y * H + ofs * i, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));
				}
			}

			if (bold) {
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						lines.draw_text(str,
							glm::vec3(-draw_aspect + x * H + ofs * j, -1.0 + y * H + ofs * i, 0.0),
							glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
							color);
					}
				}
			}
			else {
				lines.draw_text(str,
					glm::vec3(-draw_aspect + x * H, -1.0 + y * H, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					color);
			}
		};

		// Print score text
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << score;
		drawText("Time: " + stream.str() + " s", 6.8f, .4f, glm::u8vec4(0xff, 0xff, 0xff, 0xff), true);
	}
}