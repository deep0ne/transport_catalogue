#pragma once

#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"

#include <unordered_set>

class RequestHandler {
public:

    RequestHandler(const catalogue::Transport& db, const renderer::Settings& renderer);

    svg::Document RenderMap() const;

private:
    const catalogue::Transport& db_;
    const renderer::Settings& renderer_;
};

