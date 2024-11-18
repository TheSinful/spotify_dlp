#include "youtube.h"

Youtube::Youtube(AnyMetadata metadata)
{
    this->metadata = metadata;
};

bool Youtube::is_track()
{
    return std::holds_alternative<TrackMetadata>(this->metadata);
}

bool Youtube::is_album()
{
    return std::holds_alternative<AlbumMetadata>(this->metadata);
}

bool Youtube::is_playlist()
{
    return std::holds_alternative<PlaylistMetadata>(this->metadata);
}

