NAME	= webserv

CC		= c++ -Wall -Wextra -Werror -std=c++98 -pedantic -o3 -g # TODO: remove -o3 before submission

RM		= rm -rf

SRCS	= RequestParsing.cpp ResponseFormatting.cpp main.cpp WebServ.cpp \
			conf_parsing/Config.cpp conf_parsing/ConfigParse.cpp conf_parsing/DirectiveParsing.cpp \
			cgi/cgi.cpp

OBJS	= ${SRCS:.cpp=.o}

all: 	${NAME}

%.o:	%.cpp
		${CC} -c $< -o $@

${NAME}:	${OBJS}
		${CC} -o ${NAME} ${OBJS}

clean:
		${RM} ${SRCS:.cpp=.o}

fclean:		clean
		${RM} ${NAME}

re:			fclean all

.PHONY:		all clean fclean re%