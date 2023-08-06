#ifndef ErrorTracing
#define	ErrorTracing

#include <iostream>
#include <fstream>
#include <vector>

namespace ET {
	class Collector {
	private:
		inline static std::vector<std::string> _err_msgs;

	public:
		/// <summary>
		/// Adds an message to the list of messages
		/// </summary>
		/// <param name="msg">message to be added to list</param>
		/// <returns>int; to be used as Method to return one as errorcode</returns>
		static int Add(std::string msg) {
			_err_msgs.push_back(msg);
			return 1;
		}

		/// <summary>
		/// Adds an message to the list of messages. Returns errorcode directly so it can be used as method on returning line.
		/// </summary>
		/// <param name="errcode">error code to be returned from method</param>
		/// <param name="msg">message to be added to list</param>
		/// <returns>int; to be used as Method to return one as errorcode</returns>
		static int Add(const int errcode, std::string msg) {
			_err_msgs.push_back(msg);
			return errcode;
		}

		/// <summary>
		/// Adds a message and an additional list of messages to Messagelist
		/// </summary>
		/// <param name="msg"></param>
		/// <param name="msg_list"></param>
		/// <returns></returns>
		static int Add(std::string msg, std::vector<std::string> msg_list) {
			_err_msgs.push_back(msg);
			for (std::string e : msg_list) {
				_err_msgs.push_back(e);
			}
			return 1;
		}

		/// <summary>
		/// Gets list of all added messages
		/// </summary>
		/// <returns></returns>
		static std::vector<std::string> GetErrorMessages() {
			return _err_msgs;
		}

		/// <summary>
		/// Clears list of messages
		/// </summary>
		/// <returns></returns>
		static void ClearTrace() {
			_err_msgs.clear();
		}
	};

	class Logger {
	public:
		/// <summary>
		/// Writes all messages to a file
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		static int LogTraceToFile(std::string filename) {
			auto trace = Collector::GetErrorMessages();

			std::ofstream file("./" + filename + ".txt");
			for (auto line : trace) {
				line += '\n';
				file.write(line.c_str(), line.size());
			}

			return 0;
		}
	};

	class Printer {
	public:
		static int PrintTrace() {
			auto trace = Collector::GetErrorMessages();

			for (auto line : trace) {
				std::cout << line << '\n';
			}

			std::cout << std::endl;

			return 0;
		}
	};
}

#endif // !ErrorTracing