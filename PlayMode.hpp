#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>
//#include "../nest-libs/windows/glm/include/glm/glm.hpp"

#include <vector>
#include <deque>
#include <array>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const&, glm::uvec2 const& window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const& drawable_size) override;

	//----- game state -----

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//music coming from the tip of the leg (as a demonstration):
	//std::shared_ptr< Sound::PlayingSample > leg_tip_loop;

	//camera:
	Scene::Camera* camera = nullptr;

	float aspect = 1600.f / 960.f;

	enum Direction {
		dir_up,
		dir_down,
		dir_left,
		dir_right
	};

	// Directions from left to right as ordered on the note tracks
	std::array<Direction, 4> directions = {
			dir_left,
			dir_down,
			dir_right,
			dir_up
	};

	// Unit vectors corresponding to directions
	std::array<glm::vec3, 4> dir_vectors = {
		glm::vec3(-1, 0, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0)
	};

	// Input tracking:
	struct Button {
		Button(Direction dir);
		uint8_t downs = 0;
		uint8_t pressed = 0;
		Direction dir;
	};
	Button left = Button(dir_left);
	Button down = Button(dir_down);
	Button up = Button(dir_up);
	Button right = Button(dir_right);
	std::array<Button*, 4> buttons = {
		&left,
		&down,
		&right,
		&up,
	};

	// Player struct
	struct Player {
		Scene::Drawable* drawable = nullptr;
		Scene::Transform transform;
		bool alive = true;
	} player;

	// Size of the ground
	const float ground_radius = 3.f;
	Scene::Transform ground_transform;

	// Ocean properties
	std::vector<Scene::Drawable*> ocean;
	const float wave_speed = 0.25f;
	const float wave_spacing = 0.5f;
	size_t num_waves = 8;

	// Struct representing an enemy
	struct Enemy {
		Scene::Drawable* drawable = nullptr;
		Scene::Transform transform;
		bool alive = true;
	};

	// Vector of all enemies
	std::vector<Enemy*> enemies;

	// Enemy properties
	float enemy_interval = 1.25f;
	float enemy_timer = 0.f;
	const float enemy_speed = 0.1f;
	const float enemy_kill_radius = .1f;
	const float enemy_decay_speed = 0.01f;

	struct Track;
	struct Note;

	struct Track {
		Scene::Drawable* drawable = nullptr;
		Scene::Transform transform;
		std::vector<Note*> notes;
		std::vector<Note*> targets;
		float song_offset = 0;
	};

	struct Note {
		Scene::Drawable* drawable = nullptr;
		Scene::Transform transform;
		Track* track;
		Direction dir;
		bool hit = false;
	};

	// Holds the four tracks on the corners of the screen
	std::array<Track, 4> tracks;

	// Track currently being played
	Track* active_track = nullptr;

	// Describes size and spacing of various parts of the note tracks
	const float track_width = 0.3f;
	const float track_height = 0.9f;
	const float target_y = 0.4f;
	const float note_spacing = 0.1f;
	const float note_size = 0.06f;
	const float song_speed = 2.f;
	const float good_window = 0.25f;

	// Song of Water
	std::vector<int> song_of_water = {
		1, 3, 2, 0,
		3, 2, 3, 0,
		2, 4, 4, 3, 3, 3, 2, 0,
		2, 1, 3, 2, 3, 3, 1
	};

	// Song of Earth
	std::vector<int> song_of_earth = {
		2, 2, 2, 3, 4, 3, 3, 0,
		3, 2, 1, 2, 3, 3, 2, 0,
		3, 1, 4, 2, 1, 1, 2
	};

	// Song of Fire
	std::vector<int> song_of_fire = {
		3, 2, 3, 0,
		2, 1, 2, 0,
		4, 3, 4, 0,
		3, 2, 3, 0,
		1, 0,
		2, 0,
		3, 0,
		4,
	};

	// Song of Wind
	std::vector<int> song_of_wind = {
		4, 3, 2, 0,
		3, 2, 1, 0,
		4, 2, 3, 2, 3, 0,
		2, 3, 4, 3, 4
	};

	// Songs in order
	std::array<std::vector<int>, 4> songs = {
		song_of_water,
		song_of_earth,
		song_of_fire,
		song_of_wind
	};

	// Enum of songs
	enum SpellIndex {
		water_index = 0,
		earth_index = 1,
		fire_index = 2,
		wind_index = 3
	};

	// Track meshes in order
	std::array<std::string, 4> track_mesh_names = {
		"Track_Water",
		"Track_Earth",
		"Track_Fire",
		"Track_Wind"
	};

	// Song of Wind properties
	const float wind_min_distance = 0.3f;
	const float wind_distance_per_note = 0.1f;

	// Fire spell properties (adapted from game 2)
	Enemy* shooting = nullptr;
	const float shoot_duration = 0.5f;
	float shoot_timer = 0.f;
	float laser_radius = 0.05f;
	float laser_hue_amplitude = 30.f;
	float laser_hue_frequency = 3.f;
	float laser_hue_offset = 10.f;
	uint8_t num_beams = 20;

	// Earth spell properties
	struct Earth {
		Scene::Drawable* drawable;
		Scene::Transform transform;
		float timer = 0.f;
		const float duration = 0.5f;
		const float min_radius = 0.75f;
		const float radius_per_note = 0.075f;
		const float rise_height = 0.5f;
	} earth;
	std::vector<Enemy*> tossed_enemies;

	// Water spell properties
	struct Water {
		Scene::Drawable* drawable;
		Scene::Transform transform;
		float timer = 0.f;
		float radius = 0.f;
		const float min_radius = 0.75f;
		const float time_per_note = .75f;
		const float size_per_note = 0.04f;
		const float range = 1.5f;
		const float min_duration = 20.f;
		const float slowdown = 0.5f;
	} water;

	// Set to false when spell is used, reset to true every beat
	bool can_use_spell = true;

	// Time between beats
	const float beat_length = 1.f / song_speed;

	// In range [0,beat_length), represents time since last beat
	float beat = 0.f;

	// Score earned this game
	float score = 0.f;

	// Look up enemy mesh in constructor to make it easier to find and remove their drawables on reset
	static inline GLuint enemy_mesh_start = 0;

	// Helper functions
	void setMesh(Scene::Drawable* drawable, std::string mesh);
	Note* makeNote(Track* track, Direction dir, bool target = false);
	void miss(Track* track);
	void useSpell(size_t spell_index, size_t dir_index, size_t power);
	void reset();
	static bool isEnemy(const Scene::Drawable& drawable);
};
