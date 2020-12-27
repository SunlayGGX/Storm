#pragma once


namespace Storm
{
	struct GeneralGraphicConfig
	{
	public:
		GeneralGraphicConfig();

	public:
		unsigned int _wantedApplicationWidth;
		unsigned int _wantedApplicationHeight;
		int _wantedApplicationXPos;
		int _wantedApplicationYPos;
		float _fontSize;
		bool _fixNearFarPlanesWhenTranslating;
		bool _selectedParticleShouldBeTopMost;
		bool _selectedParticleForceShouldBeTopMost;
	};
}
