/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/heap.h>
#include <libc/component.h>
#include <os/vfs.h>

/* libcurl includes */
extern "C" {

#include <curl/curl.h>

}


class Main
{
	private:

		Genode::Env  &_env;
		Genode::Heap &_heap;

		Genode::Attached_rom_dataspace _config_node { _env, "config" };

		/* TODO: this breaks lwip! */
		Genode::Root_directory _root_dir { _env, _heap, _config_node.xml().sub_node("vfs") };

	public:

		explicit Main(Genode::Env &env, Genode::Heap &heap)
		:
			_env { env },
			_heap { heap }
		{ }

		void run()
		{
			::curl_global_init(CURL_GLOBAL_ALL);

			auto curl { ::curl_easy_init() };
			if (!curl) {
				Genode::error("failed to init curl");
				return;
			}

			::curl_easy_setopt(curl, CURLOPT_URL, "www.google.com");
			::curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
			::curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
			::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

			Genode::log("curl returned: ",
			            static_cast<int>(::curl_easy_perform(curl)));

			::curl_easy_cleanup(curl);

			::curl_global_cleanup();
		}
};


void Libc::Component::construct(Libc::Env &env)
{
	static Genode::Heap heap { env.ram(), env.rm() };

	static Main main { env, heap };

	Libc::with_libc([&]() {
		main.run();
	});
}
