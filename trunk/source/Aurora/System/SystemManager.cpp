#include <Aurora/System/SystemManager.h>

namespace Aurora
{
	namespace System
	{
		SystemManager SystemManager::m_SystemManager;

		SystemManager* SystemManager::Instance()
		{
			return &m_SystemManager;
		}

		void SystemManager::ConfigureDialog(pspUtilityMsgDialogParams *dialog, size_t dialog_size)
		{
		    memset(dialog, 0, dialog_size);

		    dialog->base.size = dialog_size;
		    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&dialog->base.language); // Prompt language
		    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,&dialog->base.buttonSwap); // X/O button swap

		    dialog->base.graphicsThread = 0x11;
		    dialog->base.accessThread = 0x13;
		    dialog->base.fontThread = 0x12;
		    dialog->base.soundThread = 0x10;
		}

		void SystemManager::ShowMessage(const char *message)
		{
			ConfigureDialog(&dialog, sizeof(dialog));
			dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
			dialog.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;

			strcpy(dialog.message, message);

			sceUtilityMsgDialogInitStart(&dialog);

			for(;;)
			{

				Graphics::RenderManager::InstancePtr()->StartDialog();

				switch(sceUtilityMsgDialogGetStatus()) {

				case 2:
					sceUtilityMsgDialogUpdate(1);
					break;

				case 3:
					sceUtilityMsgDialogShutdownStart();
					break;

				case 0:
					return;

				}

				Graphics::RenderManager::InstancePtr()->EndDialog();
			}
		}

		int SystemManager::ShowMessageYesNo(const char *message)
		{
			ConfigureDialog(&dialog, sizeof(dialog));
			dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
			dialog.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT;
			dialog.options |= PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS|PSP_UTILITY_MSGDIALOG_OPTION_DEFAULT_NO;

			strcpy(dialog.message, message);

			sceUtilityMsgDialogInitStart(&dialog);

			bool done = true;

			while(done)
			{
				Graphics::RenderManager::InstancePtr()->StartDialog();

				switch(sceUtilityMsgDialogGetStatus()) {

				case 2:
					sceUtilityMsgDialogUpdate(1);
					break;

				case 3:
					sceUtilityMsgDialogShutdownStart();
					break;

				case 0:
					done = false;
					break;

				}

				Graphics::RenderManager::InstancePtr()->EndDialog();
			}

			//the return value
			if(dialog.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES)
				return 1;
			else if(dialog.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_NO)
				return -1;
			else
				return 0;
		}

		int SystemManager::ShowOSK(unsigned short *descritpion,unsigned short *outtext,int maxtextinput)
		{
			//osk params
			SceUtilityOskData oskData;
			SceUtilityOskParams oskParams;
			//init osk data
			memset(&oskData, 0, sizeof(SceUtilityOskData));
			oskData.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT; // Use system default for text input
			oskData.lines = 1;
			oskData.unk_24 = 1;
			oskData.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL; // Allow all input types
			oskData.desc = descritpion;
			//oskData.intext = intext[i];
			oskData.outtextlength = maxtextinput;
			oskData.outtextlimit = 32; // Limit input to 32 characters
			oskData.outtext = outtext;

			//init osk dialog
			memset(&oskParams, 0, sizeof(SceUtilityOskParams));
			oskParams.base.size = sizeof(oskParams);
			sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &oskParams.base.language);
			sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &oskParams.base.buttonSwap);
			oskParams.base.graphicsThread = 17;
			oskParams.base.accessThread = 19;
			oskParams.base.fontThread = 18;
			oskParams.base.soundThread = 16;
			oskParams.datacount = 1;
			oskParams.data = &oskData;

			sceUtilityOskInitStart(&oskParams);

			bool done = true;

			while(done)
			{
				Graphics::RenderManager::InstancePtr()->StartDialog();

				switch(sceUtilityOskGetStatus())
				{
					case PSP_UTILITY_DIALOG_INIT:
						break;

					case PSP_UTILITY_DIALOG_VISIBLE:
						sceUtilityOskUpdate(1);
						break;

					case PSP_UTILITY_DIALOG_QUIT:
						sceUtilityOskShutdownStart();
						break;

					case PSP_UTILITY_DIALOG_FINISHED:
						break;

					case PSP_UTILITY_DIALOG_NONE:
						done = false;

					default :
						break;
				}

				Graphics::RenderManager::InstancePtr()->EndDialog();
			}

			if(oskData.result == PSP_UTILITY_OSK_RESULT_CANCELLED)
				return -1;

			return 0;
		}

		void SystemManager::ShowMessageError(const char *message,int error)
		{
			ConfigureDialog(&dialog, sizeof(dialog));
			dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_ERROR;
			dialog.options = PSP_UTILITY_MSGDIALOG_OPTION_ERROR;
			dialog.errorValue = error;
			strcpy(dialog.message, message);

			sceUtilityMsgDialogInitStart(&dialog);

			bool done = true;

			while(done)
			{

				Graphics::RenderManager::InstancePtr()->StartDialog();

				int test = sceUtilityMsgDialogGetStatus();

				switch(test)
				{

				case 2:
				sceUtilityMsgDialogUpdate(1);
				break;

				case 3:
				sceUtilityMsgDialogShutdownStart();
				break;

				case 0:
				done = false;
				break;

				}

				Graphics::RenderManager::InstancePtr()->EndDialog();
			}
		}

		void SystemManager::InputUpdate()
		{
			//new to old
			oldPadData = newPadData;
			//get input data
			sceCtrlReadBufferPositive(&newPadData, 1);
			//sceCtrlPeekBufferPositive(&newPadData, 1);
		}

		bool SystemManager::KeyPressed(PspCtrlButtons button)
		{
			if (newPadData.Buttons != oldPadData.Buttons)
			{
				if (newPadData.Buttons & button)
				{
					return true;
				}
			}
			return false;
		}

		bool SystemManager::KeyHold(PspCtrlButtons button)
		{
			if (newPadData.Buttons & button)
			{
				return true;
			}

			return false;
		}

		float SystemManager::GetAnalogX()
		{
			return (((float)newPadData.Lx - 122.5f)/122.5f);
		}

		float SystemManager::GetAnalogY()
		{
			return (((float)newPadData.Ly - 122.5f)/122.5f);
		}

		u32 SystemManager::ramAvailableLineareMax (void)
		{
			u32 size, sizeblock;
			u8 *ram;


			// Init variables
			size = 0;
			sizeblock = RAM_BLOCK;

			// Check loop
			while (sizeblock)
			{
				// Increment size
				size += sizeblock;

				// Allocate ram
				ram = (u8*)malloc(size);

				// Check allocate
				if (!(ram))
				{
					// Restore old size
					size -= sizeblock;

					// Size block / 2
					sizeblock >>= 1;
				}
				else
					free(ram);
			}

			return size;
		}

		/*int SystemManager::writer(char *data, size_t size, size_t nmemb, std::string *writerData)
		{

				if (writerData == NULL)
						return false;

				writerData->append(data, size * nmemb);

				return size * nmemb;

		}

		int SystemManager::DownloadFile(std::string url, std::string &response)
		{
			// Error buffer
			char errorBuffer[CURL_ERROR_SIZE];

			// cURL Handle
			CURL *curl = curl_easy_init();

			// cURL Code
			CURLcode res;

			// HTTP Header list
			struct curl_slist *headersList = NULL;

			// Make sure we have a valid handle
			if (curl != NULL)
			{
				// Setup error buffer
				curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

				// Set Options:

				// Allow following 'Location' headers
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

				// Set URL
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			}

			// Add User Agent to headers list
			headersList = curl_slist_append(headersList, "MyPSPApp/1.0");

			// Set Connection to close
			headersList = curl_slist_append(headersList, "Connection: Close");

			// Set encoding to ALL
			curl_easy_setopt(curl, CURLOPT_ENCODING, "");

			// Set HTTP Headers
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);

			// Set writer function
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);

			// Set write buffer
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			// Perform request
			res = curl_easy_perform(curl);

			// Error
			if (res != CURLE_OK)
			{

				// Set response to error message
				response.assign(errorBuffer);

				// Cleanup
				curl_easy_cleanup(curl);
						curl_slist_free_all(headersList);

				return -1;

			} else
			{

				// Unable to create curl connection

				// Set response to error message
				response.assign("Unable to create cURL connection.");

				// Cleanup
				curl_easy_cleanup(curl);
						curl_slist_free_all(headersList);

				return -1;

			}

			// Cleanup
			curl_easy_cleanup(curl);
			curl_slist_free_all(headersList);

			return 1;

		}*/


		void SystemManager::Init()
		{

		}

		u32 SystemManager::ramAvailable (void)
		{
			u8 **ram, **temp;
			u32 size, count, x;


			// Init variables
			ram = NULL;
			size = 0;
			count = 0;

			// Check loop
			for (;;)
			{
				// Check size entries
				if (!(count % 10))
				{
					// Allocate more entries if needed
					temp = (u8**)realloc(ram,sizeof(u8*) * (count + 10));
					if (!(temp)) break;

					// Update entries and size (size contains also size of entries)
					ram = temp;
					size += (sizeof(u8 *) * 10);
				}

				// Find max lineare size available
				x = ramAvailableLineareMax();
				if (!(x)) break;

				// Allocate ram
				ram[count] = (u8*)malloc(x);
				if (!(ram[count])) break;

				// Update variables
				size += x;
				count++;
			}

			// Free ram
			if (ram)
			{
				for (x=0;x<count;x++) free(ram[x]);
				free(ram);
			}

			return size;
		}
	}
}