struct TrackModel
{
    std::vector<Clip> clips;

    void addClip(const Clip& clip) 
    {
        clips.push_back(clip);
    }

    void removeClip(size_t index) 
    {
        if (index < clips.size()) 
        {
            clips.erase(clips.begin() + index);
        }
    }
};