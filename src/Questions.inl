static TreeGrowthData q1gw1 = {
	.spread = false,
	.apicalControl = 0.45f,
	.vigorMultiplier = 2.0f,
	.tropism = vec3(1.0f, 0.0f, 0.0f),
	.shouldShed = false,
};

static TreeGrowthData q1gw2 = {
	.spread = false,
	.apicalControl = 0.5f,
	.vigorMultiplier = 2.0f,
	.tropism = vec3(0.0f, 1.0f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData q1gw3 = {
	.spread = false,
	.apicalControl = 0.55f,
	.vigorMultiplier = 2.0f,
	.tropism = vec3(0.0f, -1.f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData q1gw4 = {
	.spread = false,
	.apicalControl = 0.45f,
	.vigorMultiplier = 2.5f,
	.tropism = vec3(0.0f, -1.0f, 0.0f),
	.shouldShed = false,
};

//----------------------------------------------------

static TreeGrowthData q2gw1 = {
	.spread = true,
	.apicalControl = 0.5f,
	.vigorMultiplier = 2.0f,
	.tropism = vec3(0.0f, -1.0f, 0.0f),
	.shouldShed = false,
};

static TreeGrowthData q2gw2 = {
	.spread = true,
	.apicalControl = 0.45f,
	.vigorMultiplier = 2.5f,
	.tropism = vec3(0.0f, 1.0f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData q2gw3 = {
	.spread = true,
	.apicalControl = 0.5f,
	.vigorMultiplier = 1.0f,
	.tropism = vec3(0.0f, -1.0f, 0.0f),
	.shouldShed = true,
};

static std::vector<glm::vec<3, uint8>> q2PresetColors = { {255, 0, 0}, {0, 255, 0} , {0, 0, 255} };

// intro

static TreeGrowthData introRed = {
	.spread = true,
	.apicalControl = 0.52f,
	.vigorMultiplier = 1.2f,
	.tropism = vec3(0.0f, 0.5f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData introGreen = {
	.spread = true,
	.apicalControl = 0.5f,
	.vigorMultiplier = 1.0f,
	.tropism = vec3(0.0f, 1.0f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData introBlue = {
	.spread = true,
	.apicalControl = 0.45f,
	.vigorMultiplier = 0.8f,
	.tropism = vec3(0.0f, -1.0f, 0.0f),
	.shouldShed = true,
};

static TreeGrowthData introBlack = {
	.vigorMultiplier = 0.0f,
	.shouldShed = true,
};

static std::vector<glm::vec<3, uint8>> introPresetColors = { {255, 0, 0}, {0, 255, 0} , {0, 0, 255}, {0,0,0} };
