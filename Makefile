CXX=g++
C_FLAGS=-g -Ofast -std=c++17 -fdiagnostics-color=always
LDFLAGS=-lsfml-graphics -lsfml-system -lsfml-window -ltgui
SRCS = Controller.cpp Model.cpp View.cpp decode_funs/load_text.cpp pugixml.cpp Logger.cpp decode_funs/decode_base64.cpp decode_funs/get_default_font_filename.cpp decode_funs/exec.cpp decode_funs/get_abs_path_to_folder.cpp AlignmentGroup.cpp datastructs/Fragment.cpp gui/BookmarkWidget.cpp gui/MyScrollablePanel.cpp gui/FloatingNote.cpp gui/SWText.cpp gui/FontDialog.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean install uninstall app_link docs build

all: run install app_link

run: $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o run

# Собирает из doxygen-файлов документацию в формате html
docs: Doxyfile doxyfiles/AlignmentGroup.dox doxyfiles/Fragment.dox doxyfiles/Logger.dox doxyfiles/tocElem.dox doxyfiles/bookmarx_file.dox doxyfiles/Groups.dox doxyfiles/mainpage.dox doxyfiles/todos.dox doxyfiles/Controller.dox doxyfiles/Lifecycle.dox doxyfiles/Model.dox doxyfiles/View.dox
	doxygen Doxyfile

# Общее правило для создания объектных файлов
%.o: %.cpp
	$(CXX) $(C_FLAGS) $< -c -o $@

# Запускать Makefile с sudo для выполнения этих целей
install: run
	chmod ugo+x run
	install -D run -t /usr/local/bin/silentwater
	chmod ugo+rw /usr/local/bin/silentwater
	cp -R -t /usr/local/bin/silentwater ./Icons
	mkdir /usr/local/bin/silentwater/themes
	cp -t /usr/local/bin/silentwater startpage.fb2
	mkdir /usr/local/bin/silentwater/bookmarks
	chmod ugo+rw /usr/local/bin/silentwater/bookmarks

uninstall:
	rm -rf /usr/local/bin/silentwater
	rm /usr/share/applications/silentwater.desktop

app_link: silentwater.desktop
	chmod +x silentwater.desktop
	cp silentwater.desktop /usr/share/applications

clean:
	rm -rf *.o