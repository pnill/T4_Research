<!--#include file="globals.asp" -->
<% 
	Response.Buffer = true;
	
	function GetQSValue(name, defval)
	{
		if (Request.QueryString(name).Count > 0)
			return Request.QueryString(name);
		return defval;
	}
	
	var plookup		= new ActiveXObject("Statsmaster.PlayerInfoLookup");
	var it			= new ActiveXObject("Statsmaster.IndexedTable");
	
	var numrows = 10;
	var offset	= numrows * parseInt( GetQSValue("page", 0) );
	var leaderboard = parseInt( GetQSValue("leaderboard", 0) );
	var category = leaderboard % NUM_ONLINE_STAT_CATEGORIES;
	var difficulty = ( leaderboard - 1 ) / NUM_ONLINE_STAT_CATEGORIES;
	
	var rows = it.GetRows( GAMEID, leaderboard, "profileid, c0, c1, c2, c3, c4", "c0 desc", numrows, offset );
	var parray = plookup.GetMultiplePlayerInfo(rows);
	var row;
	
	//add all the player names to a scripting dictionary for fast reference by profileid
	var namedict = new ActiveXObject("Scripting.Dictionary");
	
	if( parray.dimensions() > 0 )
	{
		var pinfo;
		for( row = parray.lbound(1); row <= parray.ubound(1); row++ )
		{
			pinfo = parray.getItem( row );
			namedict.add( pinfo.GetProfileID(), pinfo.GetNick() );
		}	
	}
		
	// first write out the number of rows
	numrows = rows.ubound(2) + 1;
	Response.Write( numrows );
	Response.Write( "," );

	var kv = new ActiveXObject("Statsmaster.KeyValueList");
	var ps = new ActiveXObject("Statsmaster.KVPersistStorage");
	var pid;
	for( row = rows.lbound(2); row <= rows.ubound(2); row++ )
	{
		pid = rows.getItem( PIDCOL, row );
		if( namedict.Exists(pid) ) //check if name is in our dictionary
		{
			Response.Write( namedict.Item( pid ) );		//name
		}
		else
		{
			Response.Write( "UNKNOWN" );				//name	
		}
		Response.Write( "," );
	
		// write out innings pitched(used to diplay stats per 9 innings)
		kv = ps.LoadRecord( GAMEID, pid, difficulty, 3 );
		Response.Write( kv.GetIntValue( "InningsPitched", 0 ) );	Response.Write( "," );
		
		Response.Write( rows.getItem( 0, row ) );	Response.Write( "," );
		Response.Write( rows.getItem( 1, row ) );	Response.Write( "," );
		Response.Write( rows.getItem( 2, row ) );	Response.Write( "," );
		Response.Write( rows.getItem( 3, row ) );	Response.Write( "," );
		Response.Write( rows.getItem( 4, row ) );	Response.Write( "," );
		
		if( category != LB_FIELDING_ROOKIE_6 )
		{			
			Response.Write( rows.getItem( 5, row ) );	Response.Write( "," );
		}
		
		if( category == LB_GENERAL_ROOKIE_6	|| category == LB_40_ROOKIE_6	||
			category == LB_81_ROOKIE_6		|| category == 0 /*for LEADERBOARD_162_ROOKIE_6 */ )
		{
			Response.Write( kv.GetIntValue( "RunsScored", 0 ) );	Response.Write( "," );
			Response.Write( kv.GetIntValue( "RunsAgainst", 0 ) );	Response.Write( "," );
		}
		else if( category == LB_BATTING_ROOKIE_6 )
		{
			Response.Write( kv.GetIntValue( "HomeRuns", 0 ) );	Response.Write( "," );
			Response.Write( kv.GetIntValue( "Strikeouts", 0 ) );	Response.Write( "," );
		}
		else if( category == LB_PITCHING_ROOKIE_6 )
		{
			//Response.Write( kv.GetIntValue( "Hits", 0 ) );		Response.Write( "," );
			//Response.Write( kv.GetIntValue( "InningsPitched", 0 ) );	Response.Write( "," );
		}

		//Response.Write( rows.toArray().toString() );
	}
%>