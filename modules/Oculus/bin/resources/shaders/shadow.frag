	uniform sampler2D uShadowMap;

	varying vec4 ShadowCoord;
	varying vec4 color;

	void main()
	{	
		vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w ;
		
		// Used to lower moiré pattern and self-shadowing
		shadowCoordinateWdivide.z += 0.0005;
		
		
		float distanceFromLight = texture2D(uShadowMap,shadowCoordinateWdivide.st).z;
		
		
	 	float shadow = 1.0;
	 	if (ShadowCoord.w > 0.0)
	 		shadow = distanceFromLight > shadowCoordinateWdivide.z ? 1.0 : 0.5 ;
	  	
		
		gl_FragColor =	shadow * color;
	  
	}
		