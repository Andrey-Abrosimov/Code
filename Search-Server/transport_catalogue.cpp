#include "transport_catalogue.h"

void SortByName(std::deque<std::string>& container) {
	sort(container.begin(), container.end(),
			[](const std::string& lhs, const std::string& rhs) {
				return lhs < rhs;
			}
	);
}

std::ostream& operator<<(std::ostream& out, const InfoAboutBus& bus) {
    out << "Bus " << bus.name << ": ";
    out << bus.stops_on_route << " " << "stops on route" << ", ";
    out << bus.unique_stops << " " << "unique stops" << ", ";
    out << std::setprecision(6) << bus.route_lenght << " " << "route length" << ", ";
    out << std::setprecision(6) << bus.curvature << " " << "curvature";

    return out;
}

std::ostream& operator<<(std::ostream& out, Stops_for_Bus& stop) {
    out << "Stop " << stop.name << ": buses";
    SortByName(stop.buses);
    for(auto it = stop.buses.begin(); it!=stop.buses.end(); it++)
    {
        out << " " << *it;
    }
    
    return out;
}
