#ifndef __CSJTHEMEMANAGER_H__
#define __CSJTHEMEMANAGER_H__

/**
 * CSJThemeManager provides the interfaces for managing the theme of the whole app.
 * 
 * The themes are based on QSS, and the app will provide default style sheets for 
 * default theme, and users can add and save new themes which will be saved as 
 * QSS files.
 * 
 * The Theme qss files are identified by unique names, so when adding a new theme 
 * style sheet file, there will return false if the its name has already existed.
 * 
 * Get theme style sheets for specific modules
 * 
 * Save the style sheets which are set by users.
 * 
 */

class CSJThemeManager {
public:
    CSJThemeManager() = default;
    ~CSJThemeManager();

};

#endif // __CSJTHEMEMANAGER_H__