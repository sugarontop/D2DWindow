var win = createwindow( {'x':600,'y':10,'w':300,'h':400} );var nms={};for( var i = 0; i <20; i++ ) {	nms[i] = createtextbox( { 'parent': win, 'x': 50, 'y':10 + 30*i } );	settext( nms[i], "ガビーン" + i + "号" );}

settext( nms[19], "ぐわし！" );

