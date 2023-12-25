#pragma once

#include "UIComponents.h"
#include "UIComponentsHigher.h"
#include "EWEngine/graphics/model/EWE_Basic_Model.h"
#include "EWEngine/GUI/MenuEnums.h"

#include "EWEngine/graphics/EWE_FrameInfo.h"
//#include "../../Game/InputHandler.h"

#include <queue>

namespace EWE {

	//big ol enum to key the menu??


	//holds an entire menu and handles all components
	//interaction between moduels and engine is handled in UIHandler
	class MenuModule {
	public:
		struct UIImageStruct {
			TextureID textureID{0};
			Transform2dComponent transform{};
			UIImageStruct() {}
			UIImageStruct(TextureID textureID, Transform2dComponent& transform) : textureID{ textureID }, transform{ transform } {}
		};
		static std::unique_ptr<EWEModel> model2D;
		static std::unique_ptr<EWEModel> nineUIModel;


		static void (*changeMenuStateFromMM)(uint8_t, unsigned char);
		static void changeMenuState(uint8_t menuStates, unsigned char gameState = 255) {
			changeMenuStateFromMM(menuStates, gameState);
		}

		static std::map<MenuTextureEnum, uint16_t>  textureIDs;

		static std::queue<uint16_t> clickReturns;

		static void initTextures(EWEDevice& eweDevice);

		MenuModule() {
			//printf("SHOULD NOT BE USING THE DEFAULT CONSTRUCTOR OF MENU MODULE \n");
			//throw std::runtime_error("");
		}
		//MenuModule(MenuStates menuState, float screenWidth, float screenHeight);

		~MenuModule() {
			printf("deconstructing menu module \n");
			if (childWindow) { //why am i destructing???? nothing should be moving
				childWindow->~MenuModule();
			}
		}

		static void cleanup() {
			model2D.reset();
			nineUIModel.reset();
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
		Transform2dComponent backgroundTransform{};
		/* on hold fornow
		void processClickCallbacks(double xpos, double ypos) {

			std::pair<UIComponentTypes, int16_t> returnValues = MenuModule::checkClick(xpos, ypos);
			if (returnValues.second < 0) {
				return;
			}
			switch (returnValues.first) {
			case UIT_Combobox: {
				clickComboCallback[selectedComboBox](returnValues.second);
				break;
			}
			case UIT_Dropbox: {
				printf("no drop box is currently implemented, MAJOR BUG IF HERE \n");
				throw std::exception("drop box not currently used, might need to remove");
				clickDropCallback[selectedDropBox](returnValues.second);
				break;
			}
			case UIT_ClickTextBox: {
				clickTextCallback[returnValues.second]();
				break;
			}
			case UIT_Checkbox: {
				clickCheckCallback[returnValues.second]();
				break;
			}
			default: {
				printf("this type isn't currently supported (needs support asap) : %d \n", returnValues.first);
				break;
			}
			}
		}
		*/
		virtual void processClick(double xpos, double ypos) = 0;
		std::pair<UIComponentTypes, int16_t> checkClick(double xpos, double ypos);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);

		virtual void drawText(TextOverlay* textOverlay);
		virtual void drawNewObjects();
		void drawObjects(FrameInfo2D& frameInfo);

		bool drawingNineUI() { return (clickText.size() > 0) || (comboBoxes.size() > 0) || (menuBars.size() > 0); }
		virtual void drawNewNine();
		void drawNineUI(FrameInfo2D& frameInfo);


		static std::string getInputName(int keycode);
	};
}