#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace transport;
using namespace map_renderer;
using namespace request_handler;
using namespace json;

int main()
{
    /*std::ifstream inputFile("s12_final_opentest_1.json");
    if (!inputFile.is_open())
    {
        std::cerr << "The input file could not be opened!" << std::endl;
        return 1;
    }*/

    std::vector<StatRequest> stat_request;
    RenderSettings render_settings;
    TransportCatalogue catalogue;
    RoutingSettings routing_settings;

    JSON_R json_reader;
    RequestHandler request_handler;

    json_reader = JSON_R(std::cin);
    json_reader.Parse(catalogue, stat_request, render_settings, routing_settings);

    /*std::fstream outputFileJson("test_output_json.json", std::ios::out);
    if (!outputFileJson.is_open())
    {
        std::cerr << "The output file could not be opened!" << std::endl;
        return 1;
    }*/

    request_handler = RequestHandler();
    request_handler.ExecuteQueries(catalogue, stat_request, render_settings, routing_settings);
    json::PrintDocument(request_handler.GetDocument(), std::cout);

    //outputFileJson.close();

    //inputFile.close();
}