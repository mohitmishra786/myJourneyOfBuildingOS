class BookViewer {
    constructor(container, chapters) {
        this.container = container;
        this.chapters = chapters;
        this.currentChapter = 0;
        this.currentPage = 0;
        this.isGitHubPages = window.location.hostname.includes('github.io');
        this.basePath = this.isGitHubPages ? '/myJourneyOfBuildingOS' : '';
        this.sidebarVisible = false;
        this.init();
    }

    init() {
        console.log('Initializing BookViewer');
        console.log('Total chapters:', this.chapters.length);
        this.render();
        this.loadContent();
    }

    async loadContent() {
        try {
            let currentPath = this.chapters[this.currentChapter].pages[this.currentPage].path;
            console.log('Original path:', currentPath);
            
            // Fix path resolution
            if (this.isGitHubPages) {
                currentPath = this.basePath + '/' + currentPath;
            } else {
                // For local development, use relative path
                currentPath = currentPath;
            }
            
            console.log('Loading content from:', currentPath);
            
            const response = await fetch(currentPath);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const text = await response.text();
            console.log('Content loaded, length:', text.length);
            
            // Process PlantUML diagrams before parsing markdown
            const processedText = await this.processPlantUML(text);
            const html = marked.parse(processedText);
            
            document.getElementById('content').innerHTML = html;
            
            // Apply syntax highlighting and other enhancements
            this.enhanceContent();
            this.updateNavigationState();
        } catch (error) {
            console.error('Error loading content:', error);
            document.getElementById('content').innerHTML = `
                <div class="bg-red-50 border-l-4 border-red-400 p-4 mb-4">
                    <div class="flex">
                        <div class="flex-shrink-0">
                            <svg class="h-5 w-5 text-red-400" viewBox="0 0 20 20" fill="currentColor">
                                <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd" />
                            </svg>
                        </div>
                        <div class="ml-3">
                            <h3 class="text-sm font-medium text-red-800">Error loading content</h3>
                            <div class="mt-2 text-sm text-red-700">
                                <p>${error.message}</p>
                                <p class="mt-1 text-xs">Path attempted: ${currentPath}</p>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        }
    }

    async processPlantUML(content) {
        // Find PlantUML code blocks
        const plantUMLRegex = /```plantuml\n([\s\S]*?)\n```/g;
        
        let processedContent = content;
        let match;
        
        while ((match = plantUMLRegex.exec(content)) !== null) {
            const plantUMLCode = match[1];
            
            // Use PlantUML text encoding service instead of trying to compress
            const imageUrl = this.createPlantUMLUrl(plantUMLCode);
            
            // Replace the PlantUML code block with an image
            const replacement = `<div class="plantuml-container my-8">
                <div class="bg-white p-6 rounded-lg border border-gray-200 shadow-sm">
                    <img src="${imageUrl}" alt="PlantUML Diagram" class="max-w-full h-auto mx-auto" 
                         onerror="this.style.display='none'; this.nextElementSibling.style.display='block';">
                    <div style="display:none;" class="bg-yellow-50 border border-yellow-200 rounded-lg p-4">
                        <p class="text-yellow-800 text-sm font-medium">⚠️ Could not load PlantUML diagram</p>
                        <details class="mt-2">
                            <summary class="text-xs text-yellow-600 cursor-pointer hover:text-yellow-800">Show PlantUML source code</summary>
                            <pre class="mt-2 text-xs bg-yellow-100 p-3 rounded overflow-x-auto font-mono">${plantUMLCode}</pre>
                        </details>
                    </div>
                </div>
            </div>`;
            
            processedContent = processedContent.replace(match[0], replacement);
        }
        
        return processedContent;
    }

    // Create PlantUML URL using a working encoding method
    createPlantUMLUrl(plantuml_text) {
        // Add @startuml and @enduml if not present
        let uml = plantuml_text.trim();
        if (!uml.startsWith('@startuml')) {
            uml = '@startuml\n' + uml;
        }
        if (!uml.endsWith('@enduml')) {
            uml = uml + '\n@enduml';
        }
        
        try {
            // Method 1: Try PlantUML server with simple URL encoding
            const simpleEncoded = encodeURIComponent(uml);
            const plantUMLUrl = `https://www.plantuml.com/plantuml/png/~h${this.hexEncode(uml)}`;
            return plantUMLUrl;
        } catch (e) {
            console.warn('PlantUML encoding failed, using alternative service:', e);
            try {
                // Method 2: Use plantuml-server.com as backup
                const base64 = btoa(uml);
                return `https://plantuml-server.kkeisuke.dev/svg/${base64}`;
            } catch (e2) {
                console.error('All PlantUML services failed:', e2);
                return this.createPlantUMLFallback(plantuml_text);
            }
        }
    }

    // Simple hex encoding for PlantUML
    hexEncode(text) {
        let result = '';
        for (let i = 0; i < text.length; i++) {
            result += text.charCodeAt(i).toString(16).padStart(2, '0');
        }
        return result;
    }

    // Encode for Kroki service (keeping as backup)
    encodeForKroki(text) {
        return btoa(unescape(encodeURIComponent(text)))
            .replace(/\+/g, '-')
            .replace(/\//g, '_')
            .replace(/=/g, '');
    }

    // Create a fallback SVG when PlantUML fails
    createPlantUMLFallback(originalText) {
        const lines = originalText.split('\n').slice(0, 5);
        const svgContent = `
            <svg width="500" height="300" xmlns="http://www.w3.org/2000/svg">
                <defs>
                    <pattern id="grid" width="20" height="20" patternUnits="userSpaceOnUse">
                        <path d="M 20 0 L 0 0 0 20" fill="none" stroke="#e5e7eb" stroke-width="1"/>
                    </pattern>
                </defs>
                <rect width="100%" height="100%" fill="url(#grid)"/>
                <rect x="10" y="10" width="480" height="280" fill="#f8f9fa" stroke="#dee2e6" stroke-width="2" rx="8"/>
                <text x="250" y="50" text-anchor="middle" fill="#495057" font-family="Inter, sans-serif" font-size="18" font-weight="600">
                    📊 PlantUML Diagram
                </text>
                <text x="250" y="80" text-anchor="middle" fill="#6c757d" font-family="Inter, sans-serif" font-size="14">
                    Diagram rendering is temporarily unavailable
                </text>
                <foreignObject x="30" y="120" width="440" height="120">
                    <div xmlns="http://www.w3.org/1999/xhtml" style="font-family: 'JetBrains Mono', monospace; font-size: 12px; color: #495057; background: #f1f3f4; padding: 15px; border-radius: 6px; max-height: 100px; overflow: hidden;">
                        <strong>Source Preview:</strong><br/>
                        ${lines.slice(0, 4).map(line => line.substring(0, 50) + (line.length > 50 ? '...' : '')).join('<br/>')}
                        ${lines.length > 4 ? '<br/>...' : ''}
                    </div>
                </foreignObject>
                <text x="250" y="270" text-anchor="middle" fill="#007bff" font-family="Inter, sans-serif" font-size="12">
                    💡 Click details below to view full source code
                </text>
            </svg>
        `;
        return `data:image/svg+xml;base64,${btoa(svgContent)}`;
    }

    // Remove old encoding methods
    plantUMLEncode(text) {
        // This method is no longer used
        return this.encodeForKroki(text);
    }

    // Backup encoding method
    encodePlantUMLText(text) {
        return this.encodeForKroki(text);
    }

    enhanceContent() {
        // Apply Prism.js syntax highlighting
        if (window.Prism) {
            window.Prism.highlightAll();
        }

        // Add copy buttons to code blocks
        const codeBlocks = document.querySelectorAll('pre code');
        codeBlocks.forEach(block => {
            const pre = block.parentNode;
            if (!pre.querySelector('.copy-btn')) {
                const copyBtn = document.createElement('button');
                copyBtn.className = 'copy-btn absolute top-2 right-2 px-2 py-1 text-xs bg-gray-800 dark:bg-gray-700 text-white rounded opacity-0 hover:opacity-100 transition-all duration-200 hover:bg-gray-700 dark:hover:bg-gray-600 font-mono';
                copyBtn.textContent = 'Copy';
                copyBtn.onclick = () => {
                    navigator.clipboard.writeText(block.textContent);
                    copyBtn.textContent = 'Copied!';
                    copyBtn.classList.add('bg-green-600');
                    setTimeout(() => {
                        copyBtn.textContent = 'Copy';
                        copyBtn.classList.remove('bg-green-600');
                    }, 2000);
                };
                pre.style.position = 'relative';
                pre.appendChild(copyBtn);
            }
        });

        // Add smooth scrolling to headers
        const headers = document.querySelectorAll('h1, h2, h3, h4, h5, h6');
        headers.forEach(header => {
            header.style.scrollMarginTop = '100px';
            
            // Add anchor links to headers
            if (!header.querySelector('.header-link')) {
                const anchor = document.createElement('a');
                anchor.className = 'header-link opacity-0 hover:opacity-100 transition-opacity duration-200 ml-2 text-blue-500 hover:text-blue-600 text-sm';
                anchor.innerHTML = '#';
                anchor.href = '#' + (header.id || header.textContent.toLowerCase().replace(/\s+/g, '-'));
                anchor.title = 'Link to this section';
                header.appendChild(anchor);
                
                // Show anchor on header hover
                header.addEventListener('mouseenter', () => {
                    anchor.classList.remove('opacity-0');
                    anchor.classList.add('opacity-70');
                });
                header.addEventListener('mouseleave', () => {
                    anchor.classList.add('opacity-0');
                    anchor.classList.remove('opacity-70');
                });
            }
        });

        // Add hover effects to tables
        const tables = document.querySelectorAll('table');
        tables.forEach(table => {
            table.classList.add('table-auto', 'border-collapse', 'bg-white', 'dark:bg-gray-800', 'shadow-sm', 'rounded-lg', 'overflow-hidden');
        });

        // Enhance blockquotes
        const blockquotes = document.querySelectorAll('blockquote');
        blockquotes.forEach(blockquote => {
            if (!blockquote.querySelector('.blockquote-icon')) {
                blockquote.classList.add('relative');
            }
        });

        // Add reading time estimate
        const content = document.getElementById('content');
        if (content && !content.querySelector('.reading-time')) {
            const text = content.textContent;
            const wordsPerMinute = 200;
            const words = text.trim().split(/\s+/).length;
            const readingTime = Math.ceil(words / wordsPerMinute);
            
            const readingTimeElement = document.createElement('div');
            readingTimeElement.className = 'reading-time text-sm text-gray-500 dark:text-gray-400 mb-8 flex items-center';
            readingTimeElement.innerHTML = `
                <svg class="w-4 h-4 mr-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
                ${readingTime} min read • ${words} words
            `;
            
            const firstChild = content.firstElementChild;
            if (firstChild) {
                content.insertBefore(readingTimeElement, firstChild);
            }
        }
    }

    updateNavigationState() {
        console.log('Current state:', {
            chapter: this.currentChapter,
            page: this.currentPage,
            totalChapters: this.chapters.length,
            totalPages: this.chapters[this.currentChapter].pages.length
        });

        // Update progress bar
        const totalPages = this.chapters.reduce((sum, chapter) => sum + chapter.pages.length, 0);
        let currentPageGlobal = 0;
        for (let i = 0; i < this.currentChapter; i++) {
            currentPageGlobal += this.chapters[i].pages.length;
        }
        currentPageGlobal += this.currentPage + 1;
        
        const progressPercent = (currentPageGlobal / totalPages) * 100;
        const progressBar = document.getElementById('progress-bar');
        if (progressBar) {
            progressBar.style.width = `${progressPercent}%`;
        }
    }

    toggleSidebar() {
        this.sidebarVisible = !this.sidebarVisible;
        const sidebar = document.getElementById('sidebar');
        const overlay = document.getElementById('sidebar-overlay');
        const mainContent = document.getElementById('main-content');
        
        if (this.sidebarVisible) {
            sidebar.classList.remove('-translate-x-full');
            overlay.classList.remove('hidden');
        } else {
            sidebar.classList.add('-translate-x-full');
            overlay.classList.add('hidden');
        }
    }

    goToPage(chapterIndex, pageIndex) {
        this.currentChapter = chapterIndex;
        this.currentPage = pageIndex;
        if (this.sidebarVisible) {
            this.toggleSidebar(); // Close sidebar after navigation on mobile
        }
        this.render();
        this.loadContent();
    }

    goToNextPage() {
        console.log('Attempting to go to next page');
        const currentChapterPages = this.chapters[this.currentChapter].pages.length;
        
        if (this.currentPage < currentChapterPages - 1) {
            console.log('Moving to next page in current chapter');
            this.currentPage++;
        } else if (this.currentChapter < this.chapters.length - 1) {
            console.log('Moving to first page of next chapter');
            this.currentChapter++;
            this.currentPage = 0;
        }
        
        this.render();
        this.loadContent();
    }

    goToPrevPage() {
        console.log('Attempting to go to previous page');
        if (this.currentPage > 0) {
            console.log('Moving to previous page in current chapter');
            this.currentPage--;
        } else if (this.currentChapter > 0) {
            console.log('Moving to last page of previous chapter');
            this.currentChapter--;
            this.currentPage = this.chapters[this.currentChapter].pages.length - 1;
        }
        
        this.render();
        this.loadContent();
    }

    generateTableOfContents() {
        return this.chapters.map((chapter, chapterIndex) => {
            const pages = chapter.pages.map((page, pageIndex) => {
                const isActive = chapterIndex === this.currentChapter && pageIndex === this.currentPage;
                return `
                    <li>
                        <button 
                            onclick="bookViewer.goToPage(${chapterIndex}, ${pageIndex})"
                            class="block w-full text-left px-3 py-2 text-sm rounded-lg transition-all duration-150 ${
                                isActive 
                                    ? 'bg-blue-50 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300 font-medium border-l-2 border-blue-500 dark:border-blue-400' 
                                    : 'text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-700 hover:text-gray-900 dark:hover:text-white'
                            }"
                        >
                            ${page.title}
                        </button>
                    </li>
                `;
            }).join('');

            return `
                <div class="mb-6">
                    <h3 class="font-semibold text-gray-900 dark:text-white mb-3 px-3 text-sm uppercase tracking-wider">${chapter.title}</h3>
                    <ul class="space-y-1">
                        ${pages}
                    </ul>
                </div>
            `;
        }).join('');
    }

    render() {
        const currentChapter = this.chapters[this.currentChapter];
        const currentPage = currentChapter.pages[this.currentPage];
        const isFirstPage = this.currentChapter === 0 && this.currentPage === 0;
        const isLastPage = this.currentChapter === this.chapters.length - 1 && 
                          this.currentPage === this.chapters[this.currentChapter].pages.length - 1;

        this.container.innerHTML = `
            <!-- Sidebar Overlay for Mobile -->
            <div id="sidebar-overlay" class="fixed inset-0 bg-black bg-opacity-50 z-40 lg:hidden hidden" onclick="bookViewer.toggleSidebar()"></div>
            
            <!-- Sidebar -->
            <div id="sidebar" class="fixed lg:static left-0 top-0 bottom-0 w-80 bg-white dark:bg-gray-800 shadow-xl transform -translate-x-full lg:transform-none transition-transform duration-300 ease-in-out z-50 lg:border-r lg:border-gray-200 dark:lg:border-gray-600 lg:shadow-none flex-shrink-0">
                <!-- Mobile header -->
                <div class="flex items-center justify-between p-4 border-b border-gray-200 dark:border-gray-600 lg:hidden">
                    <h2 class="text-lg font-semibold text-gray-900 dark:text-white">Table of Contents</h2>
                    <button onclick="bookViewer.toggleSidebar()" class="p-2 rounded-md text-gray-400 dark:text-gray-300 hover:text-gray-500 dark:hover:text-gray-200 hover:bg-gray-100 dark:hover:bg-gray-700">
                        <svg class="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
                        </svg>
                    </button>
                </div>
                
                <!-- Sidebar content -->
                <div class="overflow-y-auto h-full pb-20 lg:pb-4">
                    <div class="p-4">
                        <!-- Desktop header -->
                        <div class="hidden lg:block mb-8 pb-6 border-b border-gray-200 dark:border-gray-600">
                            <h1 class="text-xl font-bold text-gray-900 dark:text-white">OS Learning Journey</h1>
                            <p class="text-sm text-gray-600 dark:text-gray-300 mt-2">Operating Systems Architecture & Design</p>
                        </div>
                        ${this.generateTableOfContents()}
                    </div>
                </div>
            </div>

            <!-- Main Content Area -->
            <div id="main-content" class="flex-1 flex flex-col min-h-screen lg:min-h-0">
                <!-- Header -->
                <header class="bg-white dark:bg-gray-800 shadow-sm border-b border-gray-200 dark:border-gray-600 sticky top-0 z-30 flex-shrink-0">
                    <div class="px-4 lg:px-8 py-4">
                        <div class="flex items-center justify-between">
                            <div class="flex items-center space-x-4">
                                <button 
                                    onclick="bookViewer.toggleSidebar()"
                                    class="p-2 rounded-md text-gray-400 hover:text-gray-500 hover:bg-gray-100 dark:hover:bg-gray-700 lg:hidden"
                                >
                                    <svg class="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
                                    </svg>
                                </button>
                                <div>
                                    <h1 class="text-xl font-semibold text-gray-900 dark:text-white">${currentChapter.title}</h1>
                                    <p class="text-sm text-gray-600 dark:text-gray-300 mt-1">${currentPage.title}</p>
                                </div>
                            </div>
                            
                            <div class="flex items-center space-x-4">
                                <!-- Dark Mode Toggle -->
                                <button 
                                    onclick="toggleDarkMode()"
                                    class="dark-mode-toggle focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2 dark:focus:ring-offset-gray-800"
                                    title="Toggle dark mode"
                                ></button>
                                
                                <div class="flex items-center space-x-3">
                                    <button 
                                        onclick="bookViewer.goToPrevPage()"
                                        ${isFirstPage ? 'disabled' : ''}
                                        class="inline-flex items-center px-4 py-2 border border-gray-300 dark:border-gray-600 shadow-sm text-sm font-medium rounded-md text-gray-700 dark:text-gray-200 bg-white dark:bg-gray-700 hover:bg-gray-50 dark:hover:bg-gray-600 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                        title="Previous page"
                                    >
                                        <svg class="h-4 w-4 mr-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7" />
                                        </svg>
                                        Previous
                                    </button>
                                    
                                    <button
                                        onclick="bookViewer.goToNextPage()"
                                        ${isLastPage ? 'disabled' : ''}
                                        class="inline-flex items-center px-4 py-2 border border-transparent text-sm font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                        title="Next page"
                                    >
                                        Next
                                        <svg class="h-4 w-4 ml-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7" />
                                        </svg>
                                    </button>
                                </div>
                            </div>
                        </div>
                        
                        <!-- Progress Bar -->
                        <div class="mt-4">
                            <div class="bg-gray-200 dark:bg-gray-700 rounded-full h-2">
                                <div id="progress-bar" class="bg-blue-600 h-2 rounded-full transition-all duration-500" style="width: 0%"></div>
                            </div>
                        </div>
                    </div>
                </header>

                <!-- Content Area -->
                <main class="flex-1 overflow-y-auto bg-gray-50 dark:bg-gray-900">
                    <div class="max-w-4xl mx-auto px-4 lg:px-8 py-8">
                        <article id="content" class="prose prose-lg dark:prose-dark max-w-none bg-white dark:bg-gray-800 rounded-lg shadow-sm p-8 lg:p-12">
                            <div class="animate-pulse space-y-4">
                                <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-3/4"></div>
                                <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-1/2"></div>
                                <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-2/3"></div>
                            </div>
                        </article>
                        
                        <!-- Page Navigation Footer -->
                        <footer class="mt-8 pt-8 border-t border-gray-200 dark:border-gray-700">
                            <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                                <div class="text-sm text-gray-500 dark:text-gray-400">
                                    Page ${this.currentPage + 1} of ${currentChapter.pages.length} in this chapter
                                </div>
                                <div class="flex flex-col sm:flex-row space-y-2 sm:space-y-0 sm:space-x-4">
                                    ${!isFirstPage ? `
                                        <button 
                                            onclick="bookViewer.goToPrevPage()"
                                            class="inline-flex items-center text-sm text-blue-600 dark:text-blue-400 hover:text-blue-500 dark:hover:text-blue-300 transition-colors"
                                        >
                                            <svg class="h-4 w-4 mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7" />
                                            </svg>
                                            Previous: ${this.getPreviousPageTitle()}
                                        </button>
                                    ` : ''}
                                    
                                    ${!isLastPage ? `
                                        <button
                                            onclick="bookViewer.goToNextPage()"
                                            class="inline-flex items-center text-sm text-blue-600 dark:text-blue-400 hover:text-blue-500 dark:hover:text-blue-300 transition-colors"
                                        >
                                            Next: ${this.getNextPageTitle()}
                                            <svg class="h-4 w-4 ml-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7" />
                                            </svg>
                                        </button>
                                    ` : ''}
                                </div>
                            </div>
                        </footer>
                    </div>
                </main>
            </div>
        `;
    }

    getPreviousPageTitle() {
        if (this.currentPage > 0) {
            return this.chapters[this.currentChapter].pages[this.currentPage - 1].title;
        } else if (this.currentChapter > 0) {
            const prevChapter = this.chapters[this.currentChapter - 1];
            return prevChapter.pages[prevChapter.pages.length - 1].title;
        }
        return '';
    }

    getNextPageTitle() {
        const currentChapterPages = this.chapters[this.currentChapter].pages.length;
        
        if (this.currentPage < currentChapterPages - 1) {
            return this.chapters[this.currentChapter].pages[this.currentPage + 1].title;
        } else if (this.currentChapter < this.chapters.length - 1) {
            return this.chapters[this.currentChapter + 1].pages[0].title;
        }
        return '';
    }
}