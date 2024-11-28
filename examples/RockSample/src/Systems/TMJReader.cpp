#include "TMJReader.h"

#include <include/rapidjson/document.h>
#include <include/rapidjson/error/en.h>

#include <filesystem>
#include <cassert>
#include <fstream>
#include <array>

namespace EWE {
	namespace TMJReader {
		TileMapData ReadTMJ(std::string const& filePath) {
			rapidjson::Document document;
			assert(std::filesystem::exists(filePath));

			TileMapData ret{};

			std::ifstream inFile{ filePath, std::ios::binary };

			inFile.seekg(0, std::ios::end);
			size_t length = inFile.tellg();
			inFile.seekg(0, std::ios::beg);

			char* buffer = new char[length + 1];
			inFile.read(buffer, length);
			buffer[length] = '\0';

			document.Parse(buffer);
			if (document.HasParseError() || !document.IsObject()) {
				printf("error parsing settings at : %s \n", filePath.c_str());
				printf("error at %d : %s \n", static_cast<int32_t>(document.GetErrorOffset()), rapidjson::GetParseError_En(document.GetParseError()));
				assert(false && "failed to parse");
			}
			//assert(document.HasMember("compressionlevel") && document["compressionlevel"].IsInt());
			//int compressionLevel = document["compressionlevel"].GetInt();

			assert(document.HasMember("infinite") && document["infinite"].IsBool());
			assert(!document["infinite"].GetBool());
			bool setWidthHeight = false;

			assert(document.HasMember("tilewidth") && document.HasMember("tileheight"));
			ret.pixelsPerTileWidth = document["tilewidth"].GetInt();
			ret.pixelsPerTileHeight = document["tileheight"].GetInt();

			assert(document.HasMember("layers") && document["layers"].IsArray());
			const rapidjson::Value& layers = document["layers"];
			ret.layerCount = layers.Size();


			for (rapidjson::SizeType i = 0; i < layers.Size(); i++) {
				const rapidjson::Value& layer = layers[i];
				ret.layerData.emplace_back();

				if (layer.HasMember("height") && layer.HasMember("width")) {
					if (!setWidthHeight) {
						ret.height = layer["height"].GetInt();
						ret.width = layer["width"].GetInt();
						setWidthHeight = true;
					}
					else {
						assert(layer["height"].GetInt() == ret.height);
						assert(layer["width"].GetInt() == ret.width);
					}
				}
				assert(layer.HasMember("type") && layer["type"].IsString());

				if (memcmp(layer["type"].GetString(), "tilelayer", 10) == 0) {
					assert(layer.HasMember("data") && layer["data"].IsString());
					assert(layer.HasMember("encoding") && memcmp(layer["encoding"].GetString(), "base64", 7) == 0);
					ret.layerOrdering.push_back(TileMapData::Tile_Layer);

					std::string dataString{ layer["data"].GetString() };
					assert(dataString.size() % 4 == 0);
					auto& tileVector = ret.layerData.back().tileIDs;
					for (uint8_t tile_index = 0; tile_index < dataString.size(); tile_index += 4) {
						tileVector.push_back(dataString[tile_index] |
							dataString[tile_index + 1] << 8 |
							dataString[tile_index + 2] << 16 |
							dataString[tile_index + 3] << 24);
					}
				}
				else if (memcmp(layer["type"].GetString(), "objectgroup", 12) == 0) {
					assert(layer.HasMember("objects") && layer["objects"].IsArray());
					ret.layerOrdering.push_back(TileMapData::Object_Layer);

					const rapidjson::Value& objectData = layer["objects"];
					for (rapidjson::SizeType j = 0; j < objectData.Size(); j++) {
						if (objectData[j].HasMember("point") && (objectData[j]["point"].GetBool() == true)) {

							assert(objectData[j].HasMember("name"));
							bool foundMatching = false;
							const char* nameCStr = objectData[j]["name"].GetString();
							if (memcmp(nameCStr, "playerspawn", 12) == 0) {
								assert(objectData[j].HasMember("x"));
								assert(objectData[j].HasMember("y"));
								if (objectData[j]["x"].IsFloat()) {
									ret.spawnPoint[0] = objectData[j]["x"].GetFloat();
								}
								else if (objectData[j]["x"].IsInt()) {
									ret.spawnPoint[0] = static_cast<float>(objectData[j]["x"].GetInt());
								}
								else {
									assert(false);
								}
								if (objectData[j]["y"].IsFloat()) {
									ret.spawnPoint[1] = objectData[j]["y"].GetFloat();
								}
								else if (objectData[j]["y"].IsInt()) {
									ret.spawnPoint[1] = static_cast<float>(objectData[j]["y"].GetInt());
								}
								else {
									assert(false);
								}
							}
							else if (memcmp(nameCStr, "mobspawn", 9) == 0) {
								assert(false && "not supported");
							}
							else if (memcmp(nameCStr, "entrance", 9) == 0) {
								//temporary spawn point
							}
						}
						else if (objectData[j].HasMember("ellipse") && (objectData[j]["ellipse"].GetBool() == true)) {
							assert(false && "ellipse object type not supported");
						}
						else if (objectData[j].HasMember("polygon")) {
							assert(false && "polygon object type not spported");
						}
						else {
							if (objectData[j].HasMember("gid")) {
								//this is an image
								assert(false && "not supported");
							}
							else {
								//this is a rectangle
								assert(objectData[j].HasMember("width") && objectData[j].HasMember("x"));
								assert(objectData[j].HasMember("height") && objectData[j].HasMember("y"));

								assert(objectData[j].HasMember("name"));
								const char* nameCStr = objectData[j]["name"].GetString();
								if (memcmp(nameCStr, "deathbox", 9) == 0) {
									const float halfWidth = objectData[j]["width"].GetFloat() / 2.f;
									const float originX = objectData[j]["x"].GetFloat();
									ret.deathBox[0] = originX - halfWidth;
									ret.deathBox[2] = originX + halfWidth;

									const float halfHeight = objectData[j]["height"].GetFloat() / 2.f;
									const float originY = objectData[j]["y"].GetFloat();
									ret.deathBox[1] = originY - halfHeight;
									ret.deathBox[3] = originY + halfHeight;
								}
								else if (memcmp(nameCStr, "anchor", 7) == 0) {

								}
								else if (memcmp(nameCStr, "exit", 5) == 0) {
									//level exits, check properties for a level id or name
								}
							}
						}
					}
				}
				else {
					printf("layer type? : %s\n", layer["type"].GetString());
					assert(false);
				}


			}





			delete[] buffer;
			inFile.close();
		}
	} //namespace TMJReader
} //namespace EWE