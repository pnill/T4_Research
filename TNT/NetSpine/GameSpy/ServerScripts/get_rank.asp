<!--#include file="globals.asp" -->
<% 
	Response.Buffer = true;
	
	function GetQSValue(name, defval)
	{
		if (Request.QueryString(name).Count > 0)
			return Request.QueryString(name);
		return defval;
	}

	var it = new ActiveXObject("Statsmaster.IndexedTable");
	
	var leaderboard = parseInt( GetQSValue( "leaderboard", 1 ) );
	var searchpid = parseInt( GetQSValue( "pid", 0 ) );
	var pid = 0;
	var rowsperpage = 20;
	var startrank = 1;
	var rows = it.GetRows( GAMEID, leaderboard, "profileid, c0, c1, c2, c3, c4", "c0 desc", rowsperpage, startrank - 1 );
	var rank = 0;
	
	//if we are searching, make sure they show up!
	if (searchpid > 0)
	{
		while (1)
		{
			for (row = rows.lbound(2) ; row <= rows.ubound(2) ; row++)
			{
				pid = rows.getItem(PIDCOL, row);
				if (pid == searchpid) //found.. we are done
				{
					rank = startrank + row;
					break;
				}
			}
			
			if (pid == searchpid) //found.. we are done
			{
				rank = startrank + row;
				break;
			}
			
			// not found, check if we've gone too far..
			if (rows.ubound(2) < 0) //make sure there is at least one..
				break; //no rows
				
			// search through the next group
			startrank += rowsperpage;
			rows = it.GetRows(GAMEID, leaderboard, "profileid, c0, c1, c2, c3, c4", "c0 desc", rowsperpage, startrank - 1);
		}	
	}
	
	Response.Write( rank );
%>