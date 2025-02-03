#pragma once

#include "UIComponents.h"
#include "UIComponentsHigher.h"
#include "EWEngine/Graphics/Model/Basic_Model.h"
#include "EWEngine/GUI/MenuEnums.h"

namespace EWE {
	//holds an entire menu and handles all components
	//interaction between moduels and engine is handled in UIHandler
	class MenuModule {
	public:
		struct UIImageStruct {
			ImageID imgID{IMAGE_INVALID};
			Transform2D transform{};
			VkDescriptorSet descriptor{ VK_NULL_HANDLE };
			UIImageStruct() {}
			UIImageStruct(ImageID imgID, Transform2D& transform);
		};
		static EWEModel* model2D;


		static void (*ChangeMenuStateFromMM)(uint8_t, uint8_t);
		static void ChangeMenuState(uint8_t menuStates, uint8_t gameState = 255) {
			ChangeMenuStateFromMM(menuStates, gameState);
		}
		static std::function<void(SceneKey)> ChangeSceneFromMM; //might be more practical to use a C style func pointer

		static void initTextures();

		MenuModule() {
			//printf("SHOULD NOT BE USING THE DEFAULT CONSTRUCTOR OF MENU MODULE \n");
			//throw std::runtime_error("");
		}
		//MenuModule(MenuStates menuState, float screenWidth, float screenHeight);

		virtual ~MenuModule() {
			printf("deconstructing menu module \n");
			if (childWindow) { //why am i destructing???? nothing should be moving
				childWindow->~MenuModule();
			}
		}

		static void cleanup() {
			Deconstruct(model2D);
		}

		//thread pool functions repurposed, not sure if i need these yet?
		/*
			template<typename F, typename... Args>
			auto addClickTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
				using return_type = typename std::result_of<F(Args...)>::type;
				auto clickTask = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
				std::future<return_type> res = clickTask->get_future();
				{
					if (stop) {
						throw std::runtime_error("enqueue on stopped ThreadPool");
					}
					clickTasks.emplace([clickTask]() { (*clickTask)(); });

					// Increment the number of tasks enqueued
					++numTasksEnqueued;
				}
				condition.notify_one();
				return res;
			}
			template<typename F, typename... Args>
			void addVoidClickTask(F&& f, Args&&... args) {
				auto clickTask = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
				{
					if (stop) {
						throw std::runtime_error("enqueue on stopped ThreadPool");
					}
					clickTasks.emplace([clickTask]() { clickTask(); });

					// Increment the number of tasks enqueued
					++numTasksEnqueued;
				}
			}
		*/
		//end thread pool shit

		std::vector<Slider> sliders;
		std::vector<ComboBox> comboBoxes;
		std::vector<DropBox> dropBoxes; //difference is the main guy doesnt change on selection
		std::vector<Checkbox> checkBoxes;
		std::vector<TextStruct> labels; //text only
		std::vector<ClickTextBox> clickText; //clickable text, i.e. menu options
		std::vector<TypeBox> typeBoxes; //hold key and mouse callbacks
		std::vector<ControlBox> controlBoxes;
		std::vector<MenuBar> menuBars;

		std::vector<UIImageStruct> images;

		std::vector<std::function<void()>> callbacks{};

		//one function per UI button
		/* this is on hold for a little while
		std::vector<std::function<void(uint8_t)>> clickComboCallback;
		std::vector<std::function<void(uint8_t)>> clickDropCallback;
		std::vector<std::function<void(void)>> clickCheckCallback;
		std::vector<std::function<void(void)>> clickTextCallback;
		*/
		/*
		void clickTypeCallback() { //using for controls, idk how to handle this yet

		}
		*/
		//need something for sliders, they need a bigger redesign on the backend, not even currently working correctly
		//control boxes and menu bars not implemented yet

		//slider shit
		int8_t grabbedSlider = -1;
		int8_t callbackSlider = -1;

		double sliderMousePos = 1.;
		//float movementPerPixel = 0.f;
		//end slider shit

		bool activeChild = false;
		MenuModule* childWindow{ nullptr }; //pointer or member? probably pointer, linked list kinda thing
		//text boxes
		//std::vector<TextBox> textBoxes; //keybinds

		int8_t selectedComboBox = -1;
		int8_t selectedDropBox = -1;

		bool hasBackground = false;
		glm::vec3 backgroundColor = { .1f, .1f, .1f };
		Transform2D backgroundTransform{};
		virtual void ProcessClick(double xpos, double ypos) = 0;
		std::pair<UIComponentTypes, int16_t> CheckClick(double xpos, double ypos);

		void ResizeWindow(glm::vec2 rescalingRatio);

		virtual void DrawText();
		virtual void DrawNewObjects();
		//void drawObjects(FrameInfo2D& frameInfo);

		bool DrawingNineUI() { return (clickText.size() > 0) || (comboBoxes.size() > 0) || (menuBars.size() > 0); }
		virtual void DrawNewNine();
		virtual void DrawImages();
		//void drawNineUI(FrameInfo2D& frameInfo);


		static std::string GetInputName(int keycode);
	};
}