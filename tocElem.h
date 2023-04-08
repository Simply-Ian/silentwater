#pragma once
#include <string>

struct tocElem {
    // Элемент оглавления (TOC, table of contents)

    int page = 0;
    int depth = 0;
    std::string headline;

    tocElem(int p, std::string h, int d):
        page(p),
        headline(h),
        depth(d)
        {

        }
    bool operator == (const tocElem& cmp) const{
        return (cmp.page == this->page) && (cmp.depth == this->depth) && (cmp.headline == this->headline);
    }

    bool operator != (const tocElem& cmp) const{
        return (cmp.page != this->page) || (cmp.depth != this->depth) || (cmp.headline != this->headline);
    }
};