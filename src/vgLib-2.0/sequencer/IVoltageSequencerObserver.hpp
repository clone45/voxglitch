namespace vgLib_v2 {

    class IVoltageSequencerObserver {
    public:
        virtual void onVoltageSequencerChange(VoltageSequencer* newSequencer) = 0;
    };

}