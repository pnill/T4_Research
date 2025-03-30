<!--#include file="globals.asp" -->
<% 
	Response.Buffer = true;

	function GetQSValue(name, defval)
	{
		if (Request.QueryString(name).Count > 0)
			return Request.QueryString(name);
		return defval;
	}

	//var curpage = "http://" + Request.ServerVariables("SERVER_NAME") + Request.ServerVariables("SCRIPT_NAME");
	var pid = parseInt(GetQSValue("pid",0));
	var cid = parseInt(GetQSValue("cid",0));
	var Leaderboard = (cid * NUM_ONLINE_STAT_CATEGORIES) + 1;
	var curdate = new Date;
	var it = new ActiveXObject("Statsmaster.IndexedTable");
	var kv = new ActiveXObject("Statsmaster.KeyValueList");
	var ps = new ActiveXObject("Statsmaster.KVPersistStorage");

	kv = ps.LoadRecord( GAMEID, pid, cid, 3 );

	/*  
	the client string that sets these values looks like the following:
	x_sprintf( pBufGeneral, "\\Wins\\%u\\Losses\\%u\\Disconnects\\%u\\UserTeamRating\\%f\\OppTeamRating\\%f\\RunsScored\\%u\\RunsAgainst\\%u", 
		pGeneral->Wins, pGeneral->Losses, pGeneral->Disconnects, pGeneral->UserTeamRating, pGeneral->OppTeamRating, pGeneral->RunsScored, pGeneral->RunsAgainst );
	*/  
	
	var wins = kv.GetIntValue( "Wins", 0 );
	var losses = kv.GetIntValue( "Losses", 0 );
	var games = wins + losses;
	if( games == 40 )
	{
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 0, kv.GetIntValue( "UserRating", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 1, wins );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 2, losses );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 3, kv.GetIntValue( "Disconnects", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 4, kv.GetIntValue( "UserTeamRating", 0 ) );
		//it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_40, 4, kv.GetIntValue( "OppTeamRating", 0 ) );
		it.SetDateValue( GAMEID, pid, Leaderboard+THROUGH_40, curdate.getVarDate() );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_40, 0, kv.GetIntValue( "RunsScored", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_40, 1, kv.GetIntValue( "RunsAgainst", 0 ) );
	}
	else if( games == 81 )
	{
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 0, kv.GetIntValue( "UserRating", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 1, wins );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 2, losses );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 3, kv.GetIntValue( "Disconnects", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 4, kv.GetIntValue( "UserTeamRating", 0 ) );
		//it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_81, 4, kv.GetIntValue( "OppTeamRating", 0 ) );
		it.SetDateValue( GAMEID, pid, Leaderboard+THROUGH_81, curdate.getVarDate() );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_81, 0, kv.GetIntValue( "RunsScored", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_81, 1, kv.GetIntValue( "RunsAgainst", 0 ) );
	}
	else if( games == 162 )
	{	
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 0, kv.GetIntValue( "UserRating", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 1, wins );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 2, losses );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 3, kv.GetIntValue( "Disconnects", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 4, kv.GetIntValue( "UserTeamRating", 0 ) );
		//it.SetIntValue( GAMEID, pid, Leaderboard+THROUGH_162, 4, kv.GetIntValue( "OppTeamRating", 0 ) );
		it.SetDateValue( GAMEID, pid, Leaderboard+THROUGH_162, curdate.getVarDate() );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_162, 0, kv.GetIntValue( "RunsScored", 0 ) );
		it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+THROUGH_162, 1, kv.GetIntValue( "RunsAgainst", 0 ) );
	}
	
	it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 0, kv.GetIntValue( "UserRating", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 1, kv.GetIntValue( "Wins", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 2, kv.GetIntValue( "Losses", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 3, kv.GetIntValue( "Disconnects", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 4, kv.GetIntValue( "UserTeamRating", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+GENERAL, 4, kv.GetIntValue( "OppTeamRating", 0 ) );
	it.SetDateValue( GAMEID, pid, Leaderboard+GENERAL, curdate.getVarDate());
	
	it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS, 0, kv.GetIntValue( "RunsScored", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS, 1, kv.GetIntValue( "RunsAgainst", 0 ) );
	
	/*
	the client string that sets these values looks like the following:
	x_sprintf( pBufBatting, "\\AtBats\\%u\\Hits\\%u\\Doubles\\%u\\Triples\\%u\\HomeRuns\\%u\\Strikeouts\\%u\\RunsBattedIn\\%u\\StolenBases\\%u",
		pBatting->AtBats, pBatting->Hits, pBatting->Doubles, pBatting->Triples, pBatting->HomeRuns, pBatting->Strikeouts, pBatting->RunsBattedIn, pBatting->StolenBases );
	*/
	
	it.SetIntValue( GAMEID, pid, Leaderboard+BATTING, 0, kv.GetIntValue( "BattingRating", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+BATTING, 1, kv.GetIntValue( "AtBats", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+BATTING, 2, kv.GetIntValue( "Hits", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+BATTING, 3, kv.GetIntValue( "Doubles", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+BATTING, 4, kv.GetIntValue( "Triples", 0 ) );
	
	it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+BATTING, 0, kv.GetIntValue( "HomeRuns", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+BATTING, 1, kv.GetIntValue( "Strikeouts", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+BATTING, 2, kv.GetIntValue( "RunsBattedIn", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+BATTING, 3, kv.GetIntValue( "StolenBases", 0 ) );
	
	/*
	the client string that sets these values looks like the following:
	x_sprintf( pBufPitching, "\\EarnedRuns\\%u\\HomeRuns\\%u\\Ks\\%u\\Walks\\%u\\Hits\\%u\\InningsPitched\\%f\\Saves\\%u\\NoHitters\\%u\\PerfectGames\\%u",
		pPitching->EarnedRuns, pPitching->HomeRuns, pPitching->Strikeouts, pPitching->Walks, pPitching->Hits, pPitching->InningsPitched, pPitching->Saves, pPitching->NoHitters, pPitching->PerfectGames );
	*/

	it.SetIntValue( GAMEID, pid, Leaderboard+PITCHING, 0, kv.GetIntValue( "PitchingRating", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+PITCHING, 1, kv.GetIntValue( "EarnedRuns", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+PITCHING, 2, kv.GetIntValue( "HomeRuns", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+PITCHING, 3, kv.GetIntValue( "Ks", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+PITCHING, 4, kv.GetIntValue( "Hits", 0 ) );
	
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+PITCHING, 0, kv.GetIntValue( "InningsPitched", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+PITCHING, 1, kv.GetIntValue( "Hits", 0 ) );	
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+PITCHING, 2, kv.GetIntValue( "Saves", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+PITCHING, 3, kv.GetIntValue( "NoHitters", 0 ) );
	//it.SetIntValue( GAMEID, pid, Leaderboard+NUM_LEADERBOARDS+PITCHING, 4, kv.GetIntValue( "PerfectGames", 0 ) );

	/*
	the client string that sets these values looks like the following:
	x_sprintf( pBufFielding, "\\CaughtStealing\\%u\\DoublePlays\\%u\\Errors\\%u",
		pFielding->CaughtStealing, pFielding->DoublePlays, pFielding->Errors );
	*/
	
	it.SetIntValue( GAMEID, pid, Leaderboard+FIELDING, 0, kv.GetIntValue( "FieldingRating", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+FIELDING, 1, kv.GetIntValue( "DoublePlays", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+FIELDING, 2, kv.GetIntValue( "Errors", 0 ) );
	it.SetIntValue( GAMEID, pid, Leaderboard+FIELDING, 3, kv.GetIntValue( "CaughtStealing", 0 ) );
%>

<html>
	<head>
			<title>ASB Update Player</title>
	</head>
	<body>		
	</body>
</html>