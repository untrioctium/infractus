INFRACTUS_CONSOLE_COMMAND_IMPL(echo)
{
	BOOST_FOREACH( std::string line, args )
		CON_Out( console, "%s", line.c_str());
}

INFRACTUS_CONSOLE_COMMAND_IMPL(help)
{
	std::pair<std::string, ConsoleCommandInfo> command;
	BOOST_FOREACH( command, commandMap )
		CON_Out( console, "%s: %s", command.first.c_str(), command.second.helpText.c_str());
}

INFRACTUS_CONSOLE_COMMAND_IMPL(prompt)
{
	consolePrompt = args[0] + " ";
	setConsolePrompt();
}

INFRACTUS_CONSOLE_COMMAND_IMPL(program_info)
{
	std::string prog;
	if( args.size() == 0 ) prog = program->getName();
	else prog = args[0];
	
	if( !programManager->programExists(prog) )
	{
		CON_Out( console, "Program '%s' does not exist.", prog.c_str() );
		return;
	}
	
	CON_Out(console, "Info for program '%s':", prog.c_str());
	ProgramInfo info = programManager->getProgramInfo(prog);
	
	std::pair<std::string, std::string> v;
	BOOST_FOREACH( v, info.getInfo() )
		CON_Out(console, "%s: %s", v.first.c_str(), v.second.c_str());
		
	std::set<std::string> attributes = info.getAttributes();
	if( attributes.size() == 0 ) return;
	
	std::string attr;
	BOOST_FOREACH( std::string v, attributes )
		attr += v + " ";
	
	CON_Out(console, "attributes: %s", attr.c_str());
	
}

INFRACTUS_CONSOLE_COMMAND_IMPL(list_programs)
{
	std::vector<std::string> programs = programManager->getAvaliablePrograms();
	
	BOOST_FOREACH( std::string prog, programs )
	{
		ProgramInfo info = programManager->getProgramInfo(prog);

		std::string desc = info.getInfoField("description");
		std::string out = info.getInfoField("name");
		if( desc.size() > 0 ) out += ": " + desc;
		CON_Out( console, "%s", out.c_str() );
	}
}

INFRACTUS_CONSOLE_COMMAND_IMPL(refresh_programs)
{
	delete programManager;
	programManager = new ProgramManager("./programs/");
}

INFRACTUS_CONSOLE_COMMAND_IMPL(inspect)
{
	if( !inspectMode )
	{
		CON_Out(console, "Entering inspection mode.  Use the command 'exit' to return to the console.");
		inspectMode = true;
		
		setConsolePrompt("inspect] ");
		return;
	}
	
	std::string command = args[0];
	if( args[0] == "exit" )
	{
		CON_Out(console, "Exiting inspection mode.");
		inspectMode = false;
		setConsolePrompt();
		return;
	}
	
	try
	{
		std::string result = program->executeCommand(command);
		if( result.size() )
			CON_Out(console, result.c_str());
	}
	catch( InfractusProgramException& e )
	{
		CON_Out(console, "Error: %s\n", 
			boost::get_error_info<InfractusProgramException::errorString>(e)->c_str());
	}
}
