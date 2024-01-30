if (tempHost != "")
		{
			if	(config.checkNullID("", request.port_number))
			{
				tempHost = "";
				locations = config.getLocMap(tempHost, request.port_number);
			}
			else
			{ status_code = 404; return (""); }
			matching_host = get_match_vect(tempHost, request, locations, config);
			if (matching_host[0] == false)
			{ status_code = 404; return (""); }
			host_index = 0;
			for (it = locations.begin(); it != locEnd; it++)
			{
				locRoute = it->first;
				locRoute = locRoute.substr(0, locRoute.find(' '));
				if (matching_host[host_index + 1] == true && locRoute == request.path.substr(0, slashPos))
				{
					if (!(it->second.get_redir().empty()))
					{ status_code = 301; return (it->second.get_redir()); }
					break;
				}
				host_index++;
			}
			if (it == locEnd && locations.begin() != locEnd)
			{ status_code = 404; return (""); }
			else
			{
				std::vector<std::string> methods = it->second.get_methods_allowed();
				while (i < methods.size() && methods[i] != request.method)
					i++;
				if (i < methods.size())
					return (check_index_files(request, it, slashPos, dotPos, status_code));
				status_code = 405; return ("");
			}
		}
		else
		{ status_code = 404; return (""); }