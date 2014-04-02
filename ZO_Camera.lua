--JS: File load and execution priority
priority = -500

mode = "Follow"
buffer = 1.0
scrollSpeedX = 1.0
scrollSpeedY = 0.0
targetX = 0.0
targetY = 0.0
fixedX = 0.0
fixedY = 0.0
cameraZoom = 1.0
cameraSnapToActor = true

ui = {
	mode = { type="list", values={"Follow"}, default = "Follow", label="State" },
	cameraSnapToPlayer = { type="bool", label="Snap camera to Player at level start?", default = true },
}


function init()
	
	updateAlways = true

	if ( cameraSnapToPlayer ) then
		fixedX = g.player.rigidBody:posX()
		fixedY = g.player.rigidBody:posY()
		targetX = g.player.rigidBody:posX()
		targetY = g.player.rigidBody:posY()
	else
		fixedX = rigidBody:posX()
		fixedY = rigidBody:posY()
		targetX = rigidBody:posX()
		targetY = rigidBody:posY()
	end
	Image_setZoomScaleFactor( cameraZoom )
	g.worldWidthLeft = Image_toGameX(0)
	g.worldWidthRight = Image_toGameX( width())
end


function update( dt )
	if ( mode == "Follow" ) then
		
		targetX = (g.player.x + g.player2.x) / 2.0
		targetY = (g.player.y + g.player2.y) / 2.0
		--cameraZoom = targetX / ( g.player.x - g.player2.x )
		g.worldWidthLeft = Image_toGameX(0)
		g.worldWidthRight = Image_toGameX( width())
		
		--[[if( cameraZoom < 0 ) then
			cameraZoom = -cameraZoom
		end
		if( cameraZoom > 1.0 ) then
			cameraZoom = 1.0
		end
		
		Image_setZoomScaleFactor( cameraZoom )]]--
	end
	
	Image_setViewOffset( targetX, targetY )
end


