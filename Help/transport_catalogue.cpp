#include "transport_catalogue.h"

void SortByName(std::deque<std::string>& container) {
	sort(container.begin(), container.end(),
			[](const std::string& lhs, const std::string& rhs) {
				return lhs < rhs;
			}
	);
}
