class BookViewer {
    constructor(container, chapters) {
        this.container = container;
        this.chapters = chapters;
        this.currentChapter = 0;
        this.currentPage = 0;
        this.isGitHubPages = window.location.hostname.includes('github.io');
        this.basePath = this.isGitHubPages ? '/myJourneyOfBuildingOS' : '';
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
            
            // Adjust path for GitHub Pages
            if (this.isGitHubPages) {
                currentPath = currentPath.startsWith('/') ? currentPath : '/' + currentPath;
            } else {
                currentPath = currentPath.startsWith('./') ? currentPath : './' + currentPath;
            }
            
            console.log('Loading content from:', currentPath);
            
            const encodedPath = encodeURI(currentPath);
            const response = await fetch(encodedPath);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const text = await response.text();
            console.log('Content loaded, length:', text.length);
            
            const html = marked.parse(text);
            document.getElementById('content').innerHTML = html;
            
            this.updateNavigationState();
        } catch (error) {
            console.error('Error loading content:', error);
            document.getElementById('content').innerHTML = `
                <div class="bg-red-100 border-l-4 border-red-500 text-red-700 p-4" role="alert">
                    <p class="font-bold">Error loading content</p>
                    <p>${error.message}</p>
                </div>
            `;
        }
    }

    updateNavigationState() {
        console.log('Current state:', {
            chapter: this.currentChapter,
            page: this.currentPage,
            totalChapters: this.chapters.length,
            totalPages: this.chapters[this.currentChapter].pages.length
        });
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

    render() {
        const currentChapter = this.chapters[this.currentChapter];
        const currentPage = currentChapter.pages[this.currentPage];
        const isFirstPage = this.currentChapter === 0 && this.currentPage === 0;
        const isLastPage = this.currentChapter === this.chapters.length - 1 && 
                          this.currentPage === this.chapters[this.currentChapter].pages.length - 1;

        this.container.innerHTML = `
            <div class="max-w-4xl mx-auto p-4">
                <nav class="flex justify-between items-center mb-8">
                    <button 
                        onclick="bookViewer.goToPrevPage()"
                        ${isFirstPage ? 'disabled' : ''}
                        class="px-4 py-2 bg-blue-500 text-white rounded ${isFirstPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}"
                    >
                        ← Previous
                    </button>
                    
                    <div class="text-lg font-semibold text-center">
                        <div>Chapter ${this.currentChapter + 1}: ${currentChapter.title}</div>
                        <div class="text-sm text-gray-600 mt-1">${currentPage.title}</div>
                    </div>
                    
                    <button
                        onclick="bookViewer.goToNextPage()"
                        ${isLastPage ? 'disabled' : ''}
                        class="px-4 py-2 bg-blue-500 text-white rounded ${isLastPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}"
                    >
                        Next →
                    </button>
                </nav>

                <div id="content" class="prose max-w-none">
                    Loading...
                </div>

                <div class="mt-4 text-sm text-gray-500 text-center">
                    Page ${this.currentPage + 1} of ${currentChapter.pages.length}
                </div>
            </div>
        `;
    }
}