#pragma once

#include "EWEngine/Graphics/Model/Model.h"
#include <memory>
#include "EWEngine/Data/TransformInclude.h"
#include "EWEngine/Data/EngineDataTypes.h"


namespace EWE {

	class EWEGameObject {
	public:
		EWEGameObject() {}
		~EWEGameObject() {
		}

		EWEGameObject(const EWEGameObject&) = delete;
		EWEGameObject& operator=(const EWEGameObject&) = delete;
		
		EWEGameObject(EWEGameObject&& other) = default;
		EWEGameObject& operator=(EWEGameObject&& other) = default;

		bool drawable = true;

		TransformComponent transform{};
		
	};
}