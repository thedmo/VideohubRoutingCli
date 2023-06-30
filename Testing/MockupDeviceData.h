#include <device_data.hpp>
#include <string>

#define SOURCE_LABELS "SourceLabels";
#define DESTINATION_LABELS "DestinationLabels";
#define ROUTING "TestRouting";
#define LOCKS "TestLocks";

namespace mockup {
	std::unique_ptr<device_data> GetMockupDevice() {
		std::unique_ptr<device_data> mockup_device = std::make_unique<device_data>();
		mockup_device->ip = "127.0.0.1";
		mockup_device->name = "TestName";
		mockup_device->version = "1111";
		mockup_device->source_count = 44;
		mockup_device->destination_count = 44;
		mockup_device->source_labels = SOURCE_LABELS;
		mockup_device->destination_labels = DESTINATION_LABELS;
		mockup_device->routing = ROUTING;
		mockup_device->locks = LOCKS;

		return mockup_device;
	}
}