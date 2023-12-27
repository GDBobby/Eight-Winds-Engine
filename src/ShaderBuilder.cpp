#include "EWEngine/Data/ShaderBuilder.h"
#include "EWEngine/Data/FragmentShaderText.h"
#include "EWEngine/Data/VertexShaderText.h"
#include "EWEngine/resources/LoadingString.h"

#include <chrono>

void ShaderBlock::BatchCreateFragmentShader(std::vector<ShaderFlags> flagVector) {
	/*
	std::vector<std::pair<uint8_t, std::string>> needShader;
	for (int i = 0; i < flagVector.size(); i++) {
		std::string subPath = SHADER_PATH;
		subPath += std::to_string(flagVector[i]) + ".frag.spv";
		if (!std::filesystem::exists(subPath)) {
			needShader.push_back({ flagVector[i], subPath });
		}
	}
	spvtools::SpirvTools spirv_tools(SPV_ENV_VULKAN_1_0);
	for (int i = 0; i < needShader.size(); i++) {
		std::vector<uint32_t> spirv;
		if (!spirv_tools.Assemble(buildFragmentShader(needShader[i].first), &spirv)) {
			printf("Failed to assemble GLSL to SPIR-V \n");
			//std throw
		}
		std::ofstream file(needShader[i].second, std::ios::binary);
		if (!file) {
			printf("Failed to open file for writing \n");
			//std throw
		}
		file.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
		file.close();
	}
	*/
}

std::vector<uint32_t> ShaderBlock::getLoadingVertShader() {


	std::string subPath = SHADER_DYNAMIC_PATH;

	subPath += "loading.vert.spv";
	printf("subPath : %s \n", subPath.c_str());
	if (std::filesystem::exists(subPath)) {
		printf("reading loading vertex shader from file\n");
		std::ifstream inShader{ subPath, std::ios::binary };
		if (!inShader.is_open()) {
			printf("failed to open an existing file? \n");
			return {};
		}
		// Get the file size
		inShader.seekg(0, std::ios::end);
		std::streampos fileSize = inShader.tellg();
		inShader.seekg(0, std::ios::beg);

		// Calculate the number of uint32_t elements in the file
		std::size_t numElements = fileSize / sizeof(uint32_t);

		// Create a vector to store the file contents
		std::vector<uint32_t> shaderCodeSpirV(numElements);

		// Read the file data into the vector
		inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

		inShader.close();
		return shaderCodeSpirV;
	}
	std::vector<uint32_t> shaderCodeSpirV;
	if (SpirvHelper::LoadingVertSPV(shaderCodeSpirV)) {
		//printf("compiled shader to spv successfully \n");
	}
	else {
		printf("failed to compile loading vertex shader\n");
		//throw std run time error
	}

	return shaderCodeSpirV;
}
std::vector<uint32_t> ShaderBlock::getLoadingFragShader() {
	std::string subPath = SHADER_DYNAMIC_PATH;
	subPath += "loading.frag.spv";
	printf("subPath : %s \n", subPath.c_str());
	if (std::filesystem::exists(subPath)) {
		printf("reading loading frag shader from file\n");
		std::ifstream inShader{ subPath, std::ios::binary };
		if (!inShader.is_open()) {
			printf("failed to open an existing file? \n");
			return {};
		}
		// Get the file size
		inShader.seekg(0, std::ios::end);
		std::streampos fileSize = inShader.tellg();
		inShader.seekg(0, std::ios::beg);

		// Calculate the number of uint32_t elements in the file
		std::size_t numElements = fileSize / sizeof(uint32_t);

		// Create a vector to store the file contents
		std::vector<uint32_t> shaderCodeSpirV(numElements);

		// Read the file data into the vector
		inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

		inShader.close();
		return shaderCodeSpirV;
	}
	std::vector<uint32_t> shaderCodeSpirV;
	if (SpirvHelper::LoadingFragSPV(shaderCodeSpirV)) {
		//printf("compiled shader to spv successfully \n");
	}
	else {
		printf("failed to compile loading frag shader \n");
		throw std::runtime_error("failed to compile shader");
		//throw std run time error
	}

	return shaderCodeSpirV;


}

std::vector<uint32_t> ShaderBlock::getFragmentShader(ShaderFlags flags, bool hasBones) {
	/*
	auto print_msg_to_printf = [](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
			printf("\t SPIRV Validator : error: %s \n", m);
	};
	spvtools::SpirvTools spirv_tools(SPV_ENV_VULKAN_1_0);
	spirv_tools.SetMessageConsumer(print_msg_to_printf);
	*/
	//printf("gettingg fragment shader : %d \n", flags);
	std::string subPath = SHADER_DYNAMIC_PATH;
	subPath += std::to_string(flags);
	if (hasBones) {
		subPath += "b";
	}
	subPath += ".frag.spv";
	printf("subPath : %s \n", subPath.c_str());
	if (std::filesystem::exists(subPath)) {
		printf("reading shader from file : %ud \n", flags);
		std::ifstream inShader{ subPath, std::ios::binary };
		if (!inShader.is_open()) {
			printf("failed to open an existing file? \n");
			return {};
		}
		// Get the file size
		inShader.seekg(0, std::ios::end);
		std::streampos fileSize = inShader.tellg();
		inShader.seekg(0, std::ios::beg);

		// Calculate the number of uint32_t elements in the file
		std::size_t numElements = fileSize / sizeof(uint32_t);

		// Create a vector to store the file contents
		std::vector<uint32_t> shaderCodeSpirV(numElements);

		// Read the file data into the vector
		inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

		inShader.close();
		return shaderCodeSpirV;
	}
	std::vector<uint32_t> shaderCodeSpirV;
	if (SpirvHelper::BuildFlaggedFrag(flags, hasBones, shaderCodeSpirV)) {
		//printf("compiled shader to spv successfully \n");
	}
	else {
		printf("failed to compile shader : %d \n", flags);
		throw std::runtime_error("failed to compile shader");
		//throw std run time error
	}
	/*
	uint8_t paddingNeeded = 4 - shaderCodeSpirV.size() % 4;
	printf("padding needed, size - %d:%d \n", paddingNeeded, shaderCodeSpirV.size());
	for(int i = 0; i < paddingNeeded; i++) {
		shaderCodeSpirV.push_back(0);
	}
	*/

	return shaderCodeSpirV;
	
	
}

std::vector<uint32_t> ShaderBlock::getVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance) {


	std::string subPath = SHADER_DYNAMIC_PATH;
	if (hasNormal) {
		subPath += 'n';
	}

	subPath += std::to_string(boneCount) + ".vert.spv";
	printf("subPath : %s \n", subPath.c_str());
	if (std::filesystem::exists(subPath)) {
		printf("reading vertex shader from file : %d:%ud \n", hasNormal, boneCount);
		std::ifstream inShader{ subPath, std::ios::binary };
		if (!inShader.is_open()) {
			printf("failed to open an existing file? \n");
			return {};
		}
		// Get the file size
		inShader.seekg(0, std::ios::end);
		std::streampos fileSize = inShader.tellg();
		inShader.seekg(0, std::ios::beg);

		// Calculate the number of uint32_t elements in the file
		std::size_t numElements = fileSize / sizeof(uint32_t);

		// Create a vector to store the file contents
		std::vector<uint32_t> shaderCodeSpirV(numElements);

		// Read the file data into the vector
		inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

		inShader.close();
		return shaderCodeSpirV;
	}
	std::vector<uint32_t> shaderCodeSpirV;
	if (SpirvHelper::BuildFlaggedVert(hasNormal, boneCount, instanced, shaderCodeSpirV, largeInstance)) {
		//printf("compiled shader to spv successfully \n");
	}
	else {
		printf("failed to compile vertex shader : %d:%d \n", hasNormal, boneCount);
		//throw std run time error
	}
	/*
	uint8_t paddingNeeded = 4 - shaderCodeSpirV.size() % 4;
	printf("padding needed, size - %d:%d \n", paddingNeeded, shaderCodeSpirV.size());
	for(int i = 0; i < paddingNeeded; i++) {
		shaderCodeSpirV.push_back(0);
	}
	*/

	return shaderCodeSpirV;


}
bool ShaderBlock::SpirvHelper::BuildFlaggedFrag(ShaderFlags flags, bool hasBones, std::vector<unsigned int>& spirv) { //shader stage ALWAYS frag?
	glslang::TShader shader(EShLangFragment);
	glslang::TProgram program;
	const char* shaderStrings[1];
	TBuiltInResource Resources{};
	InitResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
	const std::string tempString = buildFragmentShader(flags, hasBones); //this won't allow the string to be destroyed right?
	shaderStrings[0] = tempString.c_str();
	shader.setStrings(shaderStrings, 1);

	// printf("parsing shader \n");
	if (!shader.parse(&Resources, 450, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		printf("shader parse failed \n");
		return false;  // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//
	//printf("linking program \n"); //what is this doing? just processing? processing what?
	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}
	//printf("compiling \n");
	glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), spirv);
	std::string shaderFileName = SHADER_DYNAMIC_PATH;
	shaderFileName += std::to_string(flags);
	if (hasBones) {
		shaderFileName += "b";
	}
	shaderFileName += ".frag.spv";
	std::ofstream outShader{ shaderFileName, std::ios::binary };

	if (outShader.is_open()) {
		printf("writing to shader location : %s \n", shaderFileName.c_str());
		outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
		outShader.close();
	}
	else {
		printf("failed to save shader \n");
	}

	return true;
}

bool ShaderBlock::SpirvHelper::LoadingVertSPV(std::vector<unsigned int>& spirv) {
	glslang::TShader shader(EShLangVertex);
	glslang::TProgram program;
	const char* shaderStrings[1];
	TBuiltInResource Resources{};
	InitResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
	std::string tempString;
	for (auto& line : EWE::LoadingVertString) {
		printf("line of loading vert : %s \n", line.c_str());
		tempString += line;
	}
	shaderStrings[0] = tempString.c_str();
	shader.setStrings(shaderStrings, 1);

	printf("parsing shader \n");
	try {
		if (!shader.parse(&Resources, 450, false, messages)) {
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			printf("shader parse failed \n");
			return false;  // something didn't work
		}
	}
	catch (const std::exception& except) {
		printf("error on parse : %s \n", except.what());
		return false;
	}
	printf("adding shader to program? \n");
	program.addShader(&shader);

	//
	// Program-level processing...
	//
	printf("linking program \n"); //what is this doing? just processing? processing what?
	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}
	//printf("compiling \n");
	glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv);
	std::string shaderFileName = SHADER_DYNAMIC_PATH;
	shaderFileName += "loading.vert.spv";


	std::ofstream outShader{ shaderFileName, std::ios::binary };

	if (outShader.is_open()) {
		printf("writing to shader location : %s \n", shaderFileName.c_str());
		outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
		outShader.close();
	}
	else {
		printf("failed to save shader \n");
	}

	return true;
}
bool ShaderBlock::SpirvHelper::LoadingFragSPV(std::vector<unsigned int>& spirv) { //shader stage ALWAYS frag?
	glslang::TShader shader(EShLangFragment);
	glslang::TProgram program;
	const char* shaderStrings[1];
	TBuiltInResource Resources{};
	InitResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
	std::string tempString;
	for (auto& line : EWE::LoadingFragString) {
		tempString += line;
	}
	shaderStrings[0] = tempString.c_str();
	shader.setStrings(shaderStrings, 1);

	// printf("parsing shader \n");
	try {
		if (!shader.parse(&Resources, 450, false, messages)) {
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			printf("shader parse failed \n");
			return false;  // something didn't work
		}
	}
	catch (const std::exception& except) {
		printf("error on parse : %s \n", except.what());
		return false;
	}

	program.addShader(&shader);

	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}
	//printf("compiling \n");
	glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), spirv);
	std::string shaderFileName = SHADER_DYNAMIC_PATH;
	shaderFileName += "loading.frag.spv";
	std::ofstream outShader{ shaderFileName, std::ios::binary };

	if (outShader.is_open()) {
		printf("writing to shader location : %s \n", shaderFileName.c_str());
		outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
		outShader.close();
	}
	else {
		printf("failed to save shader \n");
	}

	return true;
}

bool ShaderBlock::SpirvHelper::BuildFlaggedVert(bool hasNormal, uint16_t boneCount, bool instanced, std::vector<unsigned int>& spirv, bool largeInstance) { //shader stage ALWAYS frag?
	glslang::TShader shader(EShLangVertex);
	glslang::TProgram program;
	const char* shaderStrings[1];
	TBuiltInResource Resources{};
	InitResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
	const std::string tempString = buildVertexShader(hasNormal, boneCount, instanced, largeInstance); //this is a reference right?
	shaderStrings[0] = tempString.c_str();
	shader.setStrings(shaderStrings, 1);

	// printf("parsing shader \n");
	if (!shader.parse(&Resources, 450, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		printf("shader parse failed \n");
		return false;  // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//
	//printf("linking program \n"); //what is this doing? just processing? processing what?
	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}
	//printf("compiling \n");
	glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv);
	std::string shaderFileName = SHADER_DYNAMIC_PATH;
	if (hasNormal) {
		shaderFileName += 'n';
	}
	shaderFileName += std::to_string(boneCount) + ".vert.spv";


	std::ofstream outShader{ shaderFileName, std::ios::binary };

	if (outShader.is_open()) {
		printf("writing to shader location : %s \n", shaderFileName.c_str());
		outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
		outShader.close();
	}
	else {
		printf("failed to save shader \n");
	}

	return true;
}

std::string ShaderBlock::buildVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance) {
	std::string shaderString;
	if (hasNormal) {
		for (int i = 0; i < vertexTangentInput.size(); i++) {
			shaderString += vertexTangentInput[i];
		}
		for (int i = 0; i < vertexTangentOutput.size(); i++) {
			shaderString += vertexTangentOutput[i];
		}
	}
	else {
		for (int i = 0; i < vertexNNInput.size(); i++) {
			shaderString += vertexNNInput[i];
		}
		for (int i = 0; i < vertexNNOutput.size(); i++) {
			shaderString += vertexNNOutput[i];
		}
	}
	for (int i = 0; i < vertexEntry.size(); i++) {
		shaderString += vertexEntry[i];
	}
	if (instanced) {
		

		if(largeInstance){
			for (int i = 0; i < vertexInstanceBuffersFirstHalf.size(); i++) {
				shaderString += vertexInstanceBuffersFirstHalf[i];
			}
		}
		else {
			for (int i = 0; i < vertexSmallInstanceBuffersFirstHalf.size(); i++) {
				shaderString += vertexSmallInstanceBuffersFirstHalf[i];
			}
		}
		shaderString += std::to_string(boneCount) + ";";
		for (int i = 0; i < vertexInstanceBuffersSecondHalf.size(); i++) {
			shaderString += vertexInstanceBuffersSecondHalf[i];
		}
		if (hasNormal) {
			for (int i = 0; i < vertexTangentInstancingMainExit.size(); i++) {
				shaderString += vertexTangentInstancingMainExit[i];
			}
		}
		else {
			for (int i = 0; i < vertexNNInstancingMainExit.size(); i++) {
				shaderString += vertexNNInstancingMainExit[i];
			}
		}
	}
	else {
		for (int i = 0; i < vertexNoInstanceBuffers.size(); i++) {
			shaderString += vertexNoInstanceBuffers[i];
		}
		if (hasNormal) {
			for (int i = 0; i < vertexTangentNoInstancingMainExit.size(); i++) {
				shaderString += vertexTangentNoInstancingMainExit[i];
			}
		}
		else {
			for (int i = 0; i < vertexNNNoInstancingMainExit.size(); i++) {
				shaderString += vertexNNNoInstancingMainExit[i];
			}
		}
	}
#if DEBUGGING_SHADERS
	std::string debugShaderPath = "shaders\\debugging\\D_";
	debugShaderPath += std::to_string(boneCount) + ".vert";
	std::ofstream debugShader{ debugShaderPath, std::ios::trunc };
	if (!debugShader.is_open()) {
		printf("COULD NOT OPEN OR FIND DEBUG SHADER FILE \n");
	}
	//printf("inserting new lines : %d \n", shaderString.size());
	for (int i = 0; i < shaderString.size(); i++) {
		if ((shaderString[i] == ';') || (shaderString[i] == '{') || (shaderString[i] == '}')) {
			shaderString.insert(shaderString.begin() + i + 1, '\n');
			i++;
		}
	}
	//debugShader.write(shaderString.c_str(), shaderString.length());
	debugShader << shaderString;
	debugShader.close();
#endif


	return shaderString;
}

std::string ShaderBlock::buildFragmentShader(ShaderFlags flags, bool hasBones) {
	//printf("building fragment shader :%d \n", flags);
	//bool hasTangents = flags & 32; //if it has a normal map, it has tangents
	//bool hasBones = flags & 128;
	//bool instanced = ShaderFlags & 64;
	bool hasBumps = flags & 16;
	bool hasNormal = flags & 8;
	bool hasRough = flags & 4;
	bool hasMetal = flags & 2;
	bool hasAO = flags & 1;
	//building tangent frag
	std::string shaderString;
	//shaderString += version;
	if(!hasBumps){
		for (int i = 0; i < fragNNEntry.size(); i++) {
			shaderString += fragNNEntry[i];
		}
		if (hasNormal) {
			shaderString += "layout (location = 3) in vec3 fragTangentWorld;";
		}
		shaderString += fragExit;

		for (int i = 0; i < dataBindings.size(); i++) {
			shaderString += dataBindings[i];
		}
		for (int i = 0; i < functionBlock.size(); i++) {
			shaderString += functionBlock[i];
		}
		shaderString += albedoBinding[hasBones];

		if (hasNormal) {
			shaderString += normalBinding[hasBones];
		}
		if (hasRough) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal);
			shaderString += secondHalfBinding;
			shaderString += "roughSampler;";
		}

		if (hasMetal) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal + hasRough);
			shaderString += secondHalfBinding;
			shaderString += "metalSampler;";
		}
		if (hasAO) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal + hasRough + hasMetal);
			shaderString += secondHalfBinding;
			shaderString += "amOccSampler;";
		}
		if (hasNormal) {
			for (int i = 0; i < calcNormalFunction.size(); i++) {
				shaderString += calcNormalFunction[i];
			}
		}

		for (int i = 0; i < mainEntryBlock[0].size(); i++) {
			shaderString += mainEntryBlock[0][i];
		}
		for (int i = 0; i < mainSecondBlockNN.size(); i++) {
			shaderString += mainSecondBlockNN[i];
		}
		if (hasNormal) {
			shaderString += "vec3 normal = calculateNormal();";
		}
		else {
			shaderString += "vec3 normal = normalize(fragNormalWorld);";
		}
		if (hasRough) {
			shaderString += "float roughness = texture(roughSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "float roughness = 0.5;";
		}
		if (hasMetal) {
			shaderString += "float metal = texture(metalSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "float metal = 0.0f;";
		}
		for (int i = 0; i < mainThirdBlock.size(); i++) {
			shaderString += mainThirdBlock[i];
		}
		for (int i = 0; i < pointLightLoop.size(); i++) {
			shaderString += pointLightLoop[i];
		}
		for (int i = 0; i < sunCalculation.size(); i++) {
			shaderString += sunCalculation[i];
		}

		if (hasAO) {
			shaderString += "vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "vec3 ambient = vec3(0.05) * albedo;";
		}
		shaderString += "vec3 color = ambient + Lo;";
		shaderString += "color /= (color + vec3(1.0));";
		shaderString += "color = pow(color, vec3(1.0/2.2));";
		shaderString += "outColor = vec4(color, 1.0);}";
	}
	else { //if hasBumps, mostly doing a second block because bumpmap changes the uv variable name from fragTexCoord to fragTexCoord
		for (int i = 0; i < fragBumpEntry.size(); i++) {
			shaderString += fragBumpEntry[i];
		}
		shaderString += fragExit;
		for (int i = 0; i < dataBindings.size(); i++) {
			shaderString += dataBindings[i];
		}
		for (int i = 0; i < functionBlock.size(); i++) {
			shaderString += functionBlock[i];
		}
		//bump map should not have bones, but leaving it in regardless
		shaderString += albedoBinding[hasBones];
		if (hasNormal) {
			shaderString += normalBinding[hasBones];
		}
		if (hasRough) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal);
			shaderString += secondHalfBinding;
			shaderString += "roughSampler;";
		}

		if (hasMetal) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal + hasRough);
			shaderString += secondHalfBinding;
			shaderString += "metalSampler;";
		}
		if (hasAO) {
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal + hasRough + hasMetal);
			shaderString += secondHalfBinding;
			shaderString += "amOccSampler;";
		}
		if (hasBumps) { //should always be getting called here
			shaderString += firstHalfBinding[hasBones];
			shaderString += std::to_string(1 + hasNormal + hasRough + hasMetal + hasAO);
			shaderString += secondHalfBinding;
			shaderString += "bumpSampler;";
		}

		for (int i = 0; i < parallaxMapping.size(); i++) {
			shaderString += parallaxMapping[i];
		}
		//entering void main()
		for (int i = 0; i < mainEntryBlock[1].size(); i++) {
			shaderString += mainEntryBlock[1][i];
		}
		if (!hasNormal) {
			printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
			printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
			shaderString += "vec3 surfaceNormal = normalize(fragNormalWorld);";
		}
		if (hasRough) {
			shaderString += "float roughness = texture(roughSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "float roughness = 0.5;";
		}
		if (hasMetal) {
			shaderString += "float metal = texture(metalSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "float metal = 0.0f;";
		}
		for (int i = 0; i < mainThirdBlock.size(); i++) {
			shaderString += mainThirdBlock[i];
		}

		for (int i = 0; i < bumpSunCalculation.size(); i++) {
			shaderString += bumpSunCalculation[i];
		}

		if (hasAO) {
			shaderString += "vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;";
		}
		else {
			shaderString += "vec3 ambient = vec3(0.05) * albedo;";
		}
		shaderString += "vec3 color = ambient + Lo;";
		shaderString += "color /= (color + vec3(1.0));";
		shaderString += "color = pow(color, vec3(1.0/2.2));";
		shaderString += "outColor = vec4(color, 1.0);}";
	}


#if DEBUGGING_SHADERS
	std::string debugShaderPath = "shaders\\debugging\\D_";
	debugShaderPath += std::to_string(flags) + ".frag";
	std::ofstream debugShader{ debugShaderPath, std::ios::trunc};
	if (!debugShader.is_open()) {
		printf("COULD NOT OPEN OR FIND DEBUG SHADER FILE \n");
	}
	//printf("inserting new lines : %d \n", shaderString.size());
	for (int i = 0; i < shaderString.size(); i++) {
		if ((shaderString[i] == ';') || (shaderString[i] == '{') || (shaderString[i] == '}')) {
			shaderString.insert(shaderString.begin() + i + 1, '\n');
			i++;
		}
	}
	//debugShader.write(shaderString.c_str(), shaderString.length());
	debugShader << shaderString;
	debugShader.close();
#endif



	return shaderString;
}