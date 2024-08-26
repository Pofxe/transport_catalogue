#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

#include <string_view>
#include <variant>
#include <sstream>

namespace request_handler
{
    using namespace transport;
    using namespace map_renderer;
    using namespace json;
    using namespace router;

    class RequestHandler
    {
    public:

        RequestHandler() = default;

        std::optional<RouteInfo> GetRouteInfo(std::string_view start_, std::string_view end_, TransportCatalogue& catalogue_, TransportRouter& routing_) const;

        void ExecuteQueries(TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_requests_, RenderSettings& render_settings_, RoutingSettings& route_settings_);
        void ExecuteRenderMap(MapRenderer& map_catalogue_, TransportCatalogue& catalogue_) const;

        const Document& GetDocument();

    private:

        Document document; // json::

        BusQueryResult BusQuery(TransportCatalogue& catalogue_, std::string_view str_);
        StopQueryResult StopQuery(TransportCatalogue& catalogue_, std::string_view stop_name_);

        Node ExecuteMakeNodeStop(int id_request_, const StopQueryResult& query_result_);
        Node ExecuteMakeNodeBus(int id_request_, const BusQueryResult& query_result_);
        Node ExecuteMakeNodeRoute(StatRequest& recuest_, TransportCatalogue& catalogue_, TransportRouter& routing_);
        Node ExecuteMakeNodeMap(int id_request_, TransportCatalogue& catalogue_, RenderSettings render_settings_);
    };

}//end namespace request_handler