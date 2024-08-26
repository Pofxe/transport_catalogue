#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "domain.h"

namespace json
{
    using transport::TransportCatalogue;
    using map_renderer::RenderSettings;

    class JSON_R
    {
    public:

        JSON_R() = default;
        JSON_R(Document doc_);
        JSON_R(std::istream& input_);

        void Parse(TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_request_, RenderSettings& render_settings_, RoutingSettings& router_settings_);

        const Document& GetDocument() const;

    private:

        Document document; // json::

        void ParseNodeBase(const Node& root_, TransportCatalogue& catalogue_);
        void ParseNodeStat(const Node& root_, std::vector<StatRequest>& stat_request_);
        void ParseNodeRender(const Node& node_, RenderSettings& render_settings_);
        void ParseNodeRouting(const Node& node_, RoutingSettings& route_set_);
        void ParseNode(const Node& root, TransportCatalogue& catalogue_, std::vector<StatRequest>& stat_request_, RenderSettings& render_settings_, RoutingSettings& router_settings_);

        Stop ParseNodeStop(const Node& node_);
        Bus ParseNodeBus(const Node& node_, TransportCatalogue& catalogue_);
        std::vector<Distance> ParseNodeDistance(const Node& node_, TransportCatalogue& catalogue_);
    };

} // end namespace json