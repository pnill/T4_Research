<!--#include file="globals.asp" -->
<% 

	Response.Buffer = true;
	
	function GetQSValue(name, defval)
	{
		if (Request.QueryString(name).Count > 0)
			return Request.QueryString(name);
		return defval;
	}

	var curpage = "http://" + Request.ServerVariables("SERVER_NAME") + Request.ServerVariables("SCRIPT_NAME");
	var startrank = parseInt(GetQSValue("rnk",1));
	var searchname = GetQSValue("srch","");
	var searchpid = parseInt(GetQSValue("pid",-1));
	var searchrating = 0;
	var topscores = GetQSValue("topscores","");
	var rowsperpage = 20;
	var datestr = (new Date).toString();
	var it = new ActiveXObject("Statsmaster.IndexedTable");
	var plookup = new ActiveXObject("Statsmaster.PlayerInfoLookup");
	
%>

<HTML><HEAD><TITLE>All Star Baseball 2005 Rankings</TITLE>

	<style type="text/css">
	body		{ scrollbar-face-color: #666666; scrollbar-highlight-color: #333333; scrollbar-shadow-color: #222222; scrollbar-3dlight-color: #888888; scrollbar-arrow-color: #00ff00; scrollbar-track-color: #222222; scrollbar-darkshadow-color: #111111; font-family: verdana, arial, helvetica, sans-serif; font-size: 10pt; color: white; }
	a:link		{ color: #DD0000; }
	a:visited	{ color: #DD0000; }
	a:hover		{ color: #FF0000; }
	a.dark		{ color: #FFFFFF; }
	td			{ font-family: verdana, arial, helvetica, sans-serif; font-size: 10pt; color: black; background-color: #ffffff;}			
	tr.black	{ font-family: verdana, arial, helvetica, sans-serif; font-size: 10pt; color: white; background-color: #ffffff;}			
	tr.match	{ font-family: verdana, arial, helvetica, sans-serif; font-size: 10pt; color: white; background-color: #ffffff;}			
	</style>

</head>
<body bgcolor=#ffffff marginwidth=5 leftmargin=5 marginheight=10 topmargin=10>
		<table align=center width=468 cellpadding=2 cellspacing=2 border=0>
		<tr><td>
		<font size="5"><b>Rankings</b></font><br>
		<font size="4"><b><%=gamename%> Rankings</b></font><br>
		Last Updated: <i><%=datestr%></i><br>
		</td>
		<td align=right><img src="asb.jpg" height=99 width=148 border="0" alt=""></td>
		</tr>
		<tr>
		<td colspan=2>
		<font size=-2>
<!-- notice here -->
</font>
		</td>
		</tr>
		</table>
		
		<table align=center width=468 cellpadding=2 cellspacing=2 border=0>
		<tr><td bgcolor="#333333" align=center width=206>
				<b>Find a Member's Ranking</b></td>
			<td width=9>&nbsp;</td>
			<td width="233" align="CENTER" valign="TOP" bgcolor="#333333">
				<b>More Info</b></td>
		</tr>
		<tr>
			<td width="206" valign="MIDDLE">
				<form method="get">
				<b>Member Name</b><br>
				<input type="Text" name="srch" value="" size="22" maxlength="32" >
				<input type=submit value="Find">
				</form></td>
			<td width=9>&nbsp;</td>
			<td valign="TOP" align="center">
				<!-- [<a href="./"><b>List</b></a>]&nbsp;&nbsp; TBD!!- HOME link?-->
				[<a href="<%= curpage  %>" class="black"><b>Top Twenty</b></a>]&nbsp;&nbsp;
				[<a href="<%= curpage  %>?topscores=1"><b>Top Scores</b></a>]<br>
		        <a href="/gsspades/how.html">How Do Rankings Work?</a><br>
		        <a href="/gsspades/defs.html">Definitions</a>
        	</td>
		</tr>
		</table>
<%
	function WriteTableHeader(title)
	{
		%>
		<table align=center width=468 border=0 cellpadding=2 cellspacing=2>
		<tr>
			<td colspan="8" align="LEFT" valign="TOP" bgcolor="#333333">
				<font size=4><b><%= title %></b></font>
			</td>
		</tr>	
		<%
	}
	
	function WriteColumnHeader(name)
	{
	%>
		<td align="CENTER" valign="TOP">
			<b><%=name%></b>
		</td>
	<%
	}
				
	function WriteRankingCell(value)
	{
		%>
		<td align="RIGHT" nowrap><%= value %></td>
		<%
	}
	function WriteRankingTable(orderby, rowsperpage)
	{
		Response.Write("<tr class=black>");
		for (var i = 0 ; i < colnames.length ; i++)
		{
			WriteColumnHeader(colnames[i]);
		}
		Response.Write("</tr>");
		//now the core code - printing out the table rows
		if (startrank < 1)
			startrank = 1;
		var rows = it.GetRows(GAMEID, 0, "profileid, c0, c1, c2, c3, c4", orderby, rowsperpage, startrank - 1);
		//if we are searching, make sure they show up!
		if (searchpid > 0)
		{
			while (1)
			{
				for (row = rows.lbound(2) ; row <= rows.ubound(2) ; row++)
				{
					pid = rows.getItem(PIDCOL, row);
					if (pid == searchpid) //found.. we are done
						break;
					
						
				}
				if (pid == searchpid) //found.. we are done
					break;
				//else, not found, check if we've gone too far..
				if (rows.ubound(2) >= 0) //make sure there is at least one..
				{					
					var rating = rows.getItem(RATINGCOL,rows.ubound(2));
					if (rating < searchrating) //not found!
						break;					
				} else
					break; //no rows
				//else we search for then next group
				startrank += rowsperpage;
				rows = it.GetRows(GAMEID, 0, "profileid, c0, c1, c2, c3, c4", orderby, rowsperpage, startrank - 1);
			}
		
		}
		
		
		var parray = plookup.GetMultiplePlayerInfo(rows);
		var row;
		var pinfo;
		//add all the player names to a scripting dictionary for fast reference by profileid
		var namedict = new ActiveXObject("Scripting.Dictionary");

		if (parray.dimensions() > 0)
		{
			for (row = parray.lbound(1) ; row <= parray.ubound(1) ; row++)
			{
				pinfo = parray.getItem(row);
				namedict.add(pinfo.GetProfileID(), pinfo.GetNick());
			}	
		}
		//var colnames = ["Rank", "Name", "W", "L", "D", "User Team Rating", "Opp Team Rating" ];

		for (row = rows.lbound(2) ; row <= rows.ubound(2) ; row++)
		{
			pid = rows.getItem(PIDCOL, row);
			if (searchpid > 0 && pid == searchpid)
			{
				Response.Write("<tr class=match>");
			} else
				Response.Write("<tr class=black>");
			WriteRankingCell(startrank + row);				//rank
			//WriteRankingCell(rows.getItem(RATINGCOL,row));	//rating
			if (namedict.Exists(pid)) //check if name is in our dictionary
			{
				WriteRankingCell(namedict.Item(pid));		//name
			} else
				WriteRankingCell("UNKNOWN");				//name
			WriteRankingCell(rows.getItem(WINCOL,row));		//wins
			WriteRankingCell(rows.getItem(LOSSCOL,row));	//losses
			WriteRankingCell(rows.getItem(DISCOL,row));		//disconnects
			WriteRankingCell(rows.getItem(USERTEAMCOL,row));//team rating
			WriteRankingCell(rows.getItem(OPPTEAMCOL,row));	//opp team rating
			
			Response.Write("</tr>\n");
		}

		if (rows.ubound(2) < 0)
		{
			Response.Write("<tr class=black><td colspan=8 align=center>No players found</td></tr>");
		}
	}
	
	if (topscores != "") //show the top scores
	{
		WriteTableHeader("Most wins")
		WriteRankingTable("c1 desc", 3, 1)
		Response.Write("</table>");
		WriteTableHeader("Most Losses")
		WriteRankingTable("c2 desc", 3, 1)
		Response.Write("</table>");
		WriteTableHeader("Most Points")
		WriteRankingTable("c3 desc", 3, 1)
		Response.Write("</table>");
		if (colnames.length > 7) //addl col
		{
			WriteTableHeader("Most " + colnames[7])
			WriteRankingTable("c4 desc", 4, 1)
			Response.Write("</table>");
		}
		Response.End();
	}
	/*
	else if (searchpid > 0) //search for the given profileid
	{
		searchrating = it.GetIntValue(GAMEID, searchpid, 0, RATINGCOL - 1, -1);
		if (searchrating == -1) //not found!
		{
			Response.Write("No rating found for profileid " + searchpid);
			Response.End();
		}
		startrank = it.GetRowCountWhere(GAMEID, "c0 > " + searchrating);
		startrank -= 2; //show a few ahead
	}
	else if (searchname != "") //try a search
	{
		var players = plookup.PlayerSearch(0,"",searchname, "", "", "");
		if (players.ubound(1) < 0) //none found
		{
			Response.Write(" No players found named " + searchname + "");	
		} else
		{
			var pinfo;
			if (players.ubound(1) == 0) //only 1, just redirect for details
			{
				pinfo = players.getItem(0);
				Response.Redirect(curpage + "?pid=" + pinfo.GetProfileID());
				Response.End();
			}
			//else there are multiple..
			WriteTableHeader("Select a player for details")
			Response.Write("<tr class=black>");
			WriteColumnHeader("Nick");
			WriteColumnHeader("E-Mail");
			WriteColumnHeader("Firstname");
			WriteColumnHeader("Lastname");
			Response.Write("</tr>");
			for (var i = players.lbound(1) ; i <= players.ubound(1) ; i++)
			{
				pinfo = players.getItem(i);
				Response.Write("<tr class=black>");
				WriteRankingCell("<a href=\"" + curpage + "?pid=" + pinfo.GetProfileID() + "\">" + pinfo.GetNick() + "</a> ");
				WriteRankingCell(pinfo.GetEmail());
				WriteRankingCell(pinfo.GetFirstname());
				WriteRankingCell(pinfo.GetLastname());
				Response.Write("</tr>");
			}
			Response.Write("</table>");
		}
		Response.End();
	}
	*/


%>
<!-- Now the actual data table -->

		<%
		
		WriteTableHeader((startrank == 1) ? "Top Twenty Players" : "All Players");	

		WriteRankingTable("c0 desc", rowsperpage)

		%>
			
		</table>

		<table width=468 border=0 cellpadding=2 cellspacing=2 align=center>
		<tr><td width="50%" align="LEFT" valign="BOTTOM">
		<% 
			if (startrank > 1)
			{
				Response.Write("<a href=\"" + curpage + "?rnk=" + Math.max(1,startrank - rowsperpage)+ "\">Prev page</a>");
			}
		%>
		</td>
		<td width="50%" align="RIGHT" valign="BOTTOM">
		<% Response.Write("<a href=\"" + curpage + "?rnk=" + (startrank + rowsperpage)+ "\">Next page</a>"); %>
		</td></tr></table>

		
</body>
</html>