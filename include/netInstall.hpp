namespace netInstStuff {
    void InitializeServerSocket();
    void OnUnwound();
    bool OnDestinationSelected(int ourStorage);
    bool OnNSPSelected(std::string ourUrl, int ourStorage);
    std::vector<std::string> OnSelected();
    bool installNspLan ();
}