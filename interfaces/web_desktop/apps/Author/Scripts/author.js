/*******************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Affero General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* GNU Affero General Public License for more details.                          *
*                                                                              *
* You should have received a copy of the GNU Affero General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
*                                                                              *
*******************************************************************************/

Application.run = function( msg, iface )
{
	var w = new View( {
		'title'     : 'Author',
		'width'     : 960,
		'height'    : 600,
		'min-width' : 960,
		'min-height': 600,
	} );
	
	this.mainView = w;
	
	w.onClose = function()
	{
		Application.quit();
	}
	
	w.setMenuItems( [
		{
			name: i18n( 'menu_file' ),
			items: [
				{
					name: i18n( 'menu_new' ),
					command: 'new'
				},
				{
					name: i18n( 'menu_load' ),
					command: 'load'
				},
				{
					name: i18n( 'menu_save' ),
					command: 'save'
				},
				{
					name: i18n( 'menu_print' ),
					command: 'print'
				},
				{
					name: i18n( 'menu_quit' ),
					command: 'quit'
				}
			]
		},
		{
			name: i18n( 'menu_document' ),
			items: [
				{
					name: i18n( 'menu_insert_image' ),
					command: 'insertimage'
				},
				{
					name: i18n( 'menu_convert_images_inline' ),
					command: 'makeinlineimages'
				}
			]
		},
		{
			name: i18n( 'menu_preferences' ),
			items: [
				{
					name: i18n( 'menu_preferences' ),
					command: 'showprefs'
				}
			]
		}
	] );
	
	var f = new File( 'Progdir:Templates/maingui.html' );
	f.replacements = {
		'i18n_search' : i18n('i18n_search'),
		'i18n_find'   : i18n('i18n_find'),
		'i18n_select' : i18n('i18n_select')
	};
	f.onLoad = function( data )
	{
		w.setContent( data );
		if( msg.args )
		{
			w.sendMessage( { command: 'loadfiles', files: [ { Path: msg.args } ] } );
		}
	}
	f.load();
}

Application.handleKeys = function( k, e )
{
	if( e.ctrlKey )
	{
		if( k == 79 )
		{
			this.load(); 
			return true;
		}
		else if( k == 83 )
		{
			this.save(); 
			return true;
		}
		// n (doesn't work, find a different key)
		else if( k == 78 )
		{
			this.newFile();
			return true;
		}
		else if( k == 81 )
		{
			this.quit();
			return true;
		}
		// CTRL + I
		else if( k == 73 )
		{
			this.closeFile();
			return true;
		}
		console.log( k );
	}
	return false;
}

Application.newDocument = function()
{
	this.wholeFilename = '';
	this.mainView.setFlag( 'title', 'Author' );
	this.mainView.sendMessage( {
		command: 'newdocument'
	} );
}

// Prints a file
Application.print = function()
{
	if( !Application.filename ) return;
	/*if( this.printDialog ) return;
	var w = new View( {
		title: 'Print preview',
		width: 700,
		height: 800
	} );
	this.printDialog = w;
	*/
	Application.mainView.sendMessage( {
		command: 'print',
		path: Application.filename
	} );
}

// Loads a file
Application.load = function()
{
	if( this.fileDialog ) return;
	var f = new Filedialog( this.mainView, function( arr )
	{
		Application.mainView.sendMessage( {
			command: 'loadfiles',
			files: arr
		} );
		Application.wholeFilename = arr[0].Path;
		Application.mainView.setFlag( 'title', 'Author - ' + Application.wholeFilename );
		Application.fileDialog = false;
	}, false, 'load' );
	this.fileDialog = f;
}

// Saves current file
Application.save = function()
{
	if( this.wholeFilename )
	{
		Application.mainView.sendMessage( {
			command: 'savefile',
			path: Application.wholeFilename
		} );
	}
	else
	{
		var f = new Filedialog( this.mainView, function( fname )
		{
			Application.mainView.sendMessage( {
				command: 'savefile',
				path: fname
			} );
			Application.wholeFilename = fname;
			Application.mainView.setFlag( 'title', 'Author - ' + fname );
		}, '', 'save' );
	}
}

Application.insertImage = function( file )
{
	this.mainView.sendMessage( {
		command: 'insertimage',
		path: file
	} );
}

Application.showPrefs = function()
{
	if( this.pwin ) return;
	this.pwin = new View( {
		title: 'Preferences',
		width: 800,
		height: 500,
		id: 'authorprefswin'
	} );
	
	this.pwin.onClose = function()
	{
		Application.pwin = false;
	}
	
	var f = new File( 'Progdir:Templates/preferences.html' );
	f.onLoad = function( data )
	{
		Application.pwin.setContent( data );
	}
	f.load();
}

Application.receiveMessage = function( msg )
{
	if( !msg.command ) return;
	switch( msg.command )
	{
		case 'closeprefs':
			this.pwin.close();
			this.pwin = false;
			break;
		case 'currentfile':
			this.fileName = msg.filename;
			this.path = msg.path;
			this.wholeFilename = msg.path + msg.filename;
			this.mainView.setFlag( 'title', 'Author - ' + this.wholeFilename );
			break;
		case 'openfile':
			this.load();
			break;
		case 'keydown':
			this.handleKeys( msg.key, { ctrlKey: msg.ctrlKey } );
			break;
		case 'quit':
			this.quit();
			break;
		case 'new':
			this.newDocument();
			break;
		case 'print':
			this.print();
			break;
		case 'syncload':
			if( msg.filename )
			{
				this.wholeFilename = msg.filename;
				this.mainView.setFlag( 'title', 'Author - ' + this.wholeFilename );
			}
			break;
		case 'load':
			this.load();
			break;
		case 'save':
			this.save();
			break;
		// Make sure we can activate window
		case 'activate':
			this.mainView.activate();
			break;
		// Show preferences
		case 'showprefs':
			this.showPrefs();
			break;
		case 'makeinlineimages':
			this.mainView.sendMessage( msg );
			break;
		case 'insertimage':
			var f = new Filedialog( this.mainView, function( items )
			{
				for( var a = 0; a < items.length; a++ )
				{
					Application.insertImage( items[a].Path );
				}
			}, 'Mountlist:', 'load' );
			break;
	}
}


