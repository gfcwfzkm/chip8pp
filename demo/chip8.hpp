#ifndef _CHIP8_HPP_
#define _CHIP8_HPP_

#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <thread>
#include <chrono>
#include <map>
#include <optional>
#include "cpu.hpp"
#include "Instructions/Instruction.hpp"
#include "ch8_platform_specific.h"

namespace CHIP8Demo
{
	/**
	 * @brief Keyboard
	 * 
	 * This class represents a keyboard.
	 */
	class Keyboard : public CHIP8::Keypad
	{
		/** @brief Key Map
		 * 
		 * This map stores the key mapping.
		 * The key mapping is used to map the console keys to the CHIP-8 keys.
		 */
		std::map<int, enum Key> KeyMap;
	public:
		Keyboard(bool Chip8Keyboard = false) : CHIP8::Keypad()
		{
			// Initialize the key map
			if (Chip8Keyboard)
			{
				KeyMap.insert({ '1', Key::KEY_1 });
				KeyMap.insert({ '2', Key::KEY_2 });
				KeyMap.insert({ '3', Key::KEY_3 });
				KeyMap.insert({ '4', Key::KEY_4 });
				KeyMap.insert({ '5', Key::KEY_5 });
				KeyMap.insert({ '6', Key::KEY_6 });
				KeyMap.insert({ '7', Key::KEY_7 });
				KeyMap.insert({ '8', Key::KEY_8 });
				KeyMap.insert({ '9', Key::KEY_9 });
				KeyMap.insert({ '0', Key::KEY_0 });
				KeyMap.insert({ 'a', Key::KEY_A });
				KeyMap.insert({ 'b', Key::KEY_B });
				KeyMap.insert({ 'c', Key::KEY_C });
				KeyMap.insert({ 'd', Key::KEY_D });
				KeyMap.insert({ 'e', Key::KEY_E });
				KeyMap.insert({ 'f', Key::KEY_F });
			}
			else
			{
				KeyMap.insert({ '1', Key::KEY_1 });
				KeyMap.insert({ '2', Key::KEY_2 });
				KeyMap.insert({ '3', Key::KEY_3 });
				KeyMap.insert({ 'q', Key::KEY_4 });
				KeyMap.insert({ 'w', Key::KEY_5 });
				KeyMap.insert({ 'e', Key::KEY_6 });
				KeyMap.insert({ 'a', Key::KEY_7 });
				KeyMap.insert({ 's', Key::KEY_8 });
				KeyMap.insert({ 'd', Key::KEY_9 });
				KeyMap.insert({ 'x', Key::KEY_0 });
				KeyMap.insert({ 'y', Key::KEY_A });
				KeyMap.insert({ 'c', Key::KEY_B });
				KeyMap.insert({ '4', Key::KEY_C });
				KeyMap.insert({ 'r', Key::KEY_D });
				KeyMap.insert({ 'f', Key::KEY_E });
				KeyMap.insert({ 'v', Key::KEY_F });
			}
		}

		/**
		 * @brief IsKeyPressed
		 * 
		 * This function checks if a key is pressed.
		 * 
		 * @param key The key to check.
		 * @return bool : true if the key is pressed, false otherwise.
		 */
		bool IsKeyPressed(enum Key key) override
		{
			if (keys[key])
			{
				keys[key] = false;
				return true;
			}
			return false;
		}

		/**
		 * @brief WaitForKeyPress
		 * 
		 * This function waits for a key to be pressed.
		 * 
		 * @return enum Key : The key that was pressed.
		 */
		enum Key WaitForKeyPress() override
		{
			for (int i = 0; i < 16; i++)
			{
				if (keys[i])
				{
					keys[i] = false;
					return static_cast<enum Key>(i);
				}
			}
			UpdateKeys();
						
			return Key::KEY_INVALID;
		}

		/**
		 * @brief UpdateKeys
		 * 
		 * This function updates the keys.
		 */
		void UpdateKeys() override
		{
			if (CH8_KBHIT())
			{
				const int key = CH8_GETCH();
				if (KeyMap.find(key) != KeyMap.end())
				{
					keys[KeyMap.at(key)] = true;
				}
				// Check if the key variable is zero
			}
		}
	};
	
	/**
	 * @brief Display
	 * 
	 * This class represents a display.
	 */
	class Display : public CHIP8::Display
	{
		// Psuedo beeper, will be used since we can't beep with the console nicely
		bool beep = false;
	public:
		/**
		 * @brief Set the Beep object
		 * 
		 * @param value The value to set the beep to.
		 */
		void setBeep(bool value)
		{
			beep = value;
		}

		/**
		 * @brief Update the display
		 * 
		 * This function updates the display and is required to
		 * implement the CHIP8::Display interface.
		 */
		void Update() override
		{
			Update(beep);
		}

		/**
		 * @brief Update the display on the console
		 * 
		 * This function updates the display with the current state of the display buffer 
		 * on the console. It uses the following characters to represent the display:
		 */
		void Update(bool optionalFakeBeep = false)
		{
			// Screen border characters
			enum screenBorderType {TOP_LEFT = 0, TOP, TOP_RIGHT, LEFT, RIGHT, BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT};
			// Thicker border for the BEEP mode
			const std::array<std::string,8> screenBorderCharsBEEP = {
				CH8_FRAMEB_UPL,
				CH8_FRAMEB_UP,
				CH8_FRAMEB_UPR,
				CH8_FRAMEB_LEFT,
				CH8_FRAMEB_RGHT,
				CH8_FRAMEB_DNL,
				CH8_FRAMEB_DOWN,
				CH8_FRAMEB_DNR
			};
			// Normal border
			const std::array<std::string,8> screenBorderCharsNORMAL = {
				CH8_FRAME_UPL,
				CH8_FRAME_UP,
				CH8_FRAME_UPR,
				CH8_FRAME_LEFT,
				CH8_FRAME_RGHT,
				CH8_FRAME_DNL,
				CH8_FRAME_DOWN,
				CH8_FRAME_DNR
			};

			// Screen buffer
			std::string textScreen;

			// Are we beepin'?
			const auto &screenBorder = optionalFakeBeep ? screenBorderCharsBEEP : screenBorderCharsNORMAL;

			// Draw the upper border
			textScreen.append(screenBorder[TOP_LEFT]);
			for (int i = 0; i < Width; i++)
				textScreen.append(screenBorder[TOP]);
			textScreen.append(screenBorder[TOP_RIGHT]);
			textScreen.append("\n");

			// Screen drawing loop, rendering two vertical pixels per console line
			for (int y = 0; y < Height; y += 2)
			{
				// Draw the left border
				textScreen.append(screenBorder[LEFT]);
				for (int x = 0; x < Width; x++)
				{
					// Draw the pixel using the following characters: █▀▄ 
					if ((screenBuffer[y * Width + x]) && (screenBuffer[(y+1) * Width + x]))
					{
						textScreen.append(CH8_BOTHPIXEL);
					}
					else if (screenBuffer[y * Width + x])
					{
						textScreen.append(CH8_UPPERPIXEL);
					}
					else if (screenBuffer[(y+1) * Width + x])
					{
						textScreen.append(CH8_LOWERPIXEL);
					}
					else
					{
						textScreen.append(CH8_NOPIXEL);
					}
				}

				// Draw the right border
				textScreen.append(screenBorder[RIGHT]);
				textScreen.append("\n");
			}

			// Draw the lower border
			textScreen.append(screenBorder[BOTTOM_LEFT]);
			for (int i = 0; i < Width; i++)
				textScreen.append(screenBorder[BOTTOM]);
			textScreen.append(screenBorder[BOTTOM_RIGHT]);

			// Move the cursor to the top left corner
			std::cout << "\033[0;0H";
			// Print the screen buffer
			std::cout << textScreen;

			// Clear the "Update Screen" flag
			UpdateRequired = false;
		}
	};

	/**
	 * @brief Chip8Test
	 * 
	 * This class represents a CHIP-8 test.
	 * It is a basic example on how this CHIP8 library can be used by using the 
	 * default terminal as both screen and keyboard input.
	 */
	class Chip8Test
	{
		/**
		 * @brief Display
		 * 
		 * This variable represents the display of the CHIP-8 system.
		 */
		std::shared_ptr<Display> display;

		/**
		 * @brief Keyboard
		 * 
		 * This variable represents the keyboard of the CHIP-8 system.
		 */
		std::shared_ptr<Keyboard> keyboard;
		
		/**
		 * @brief CPU
		 * 
		 * This variable represents the CPU of the CHIP-8 system.
		 */
		CHIP8::CPU cpu;
	public:
		/**
		 * @brief Chip8Test
		 * 
		 * This constructor initializes the CHIP-8 test.
		 */
		Chip8Test();

		/**
		 * @brief Load a ROM file
		 * 
		 * This function loads a ROM file.
		 * 
		 * @param filename The filename of the ROM file.
		 * @return std::expected<void, std::string> : An expected object with an error message if the ROM file could not be loaded.
		 */
		std::optional<std::string> loadRom(const std::string &filename);

		/**
		 * @brief Play the ROM file
		 * 
		 * This function plays the ROM file.
		 */
		void playRom();
	};
}

#endif /* _CHIP8_HPP_ */